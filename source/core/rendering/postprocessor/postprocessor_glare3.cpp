#include "postprocessor_glare3.hpp"
#include "image/typed_image.inl"
#include "base/memory/align.hpp"
#include "base/math/exp.hpp"
#include "base/math/vector.inl"
#include "base/math/vector4.inl"
#include "base/math/filter/gaussian.hpp"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/spectrum/interpolated.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/spectrum/xyz.hpp"
#include "base/random/generator.inl"
#include "base/thread/thread_pool.hpp"
#include <vector>

#include "base/math/print.hpp"
#include <iostream>

namespace rendering { namespace postprocessor {

Glare3::Glare3(Adaption adaption, float threshold, float intensity) :
	Postprocessor(4),
	adaption_(adaption),
	threshold_(threshold), intensity_(intensity),
	high_pass_(nullptr),
	kernel_(nullptr),
	gauss_kernel_(nullptr) {}

Glare3::~Glare3() {
	memory::free_aligned(gauss_kernel_);
	memory::free_aligned(kernel_);
	memory::free_aligned(high_pass_);
}

static inline float f0(float theta) {
	float b = theta / 0.02f;
	return 2.61f * 10e6f * math::exp(-(b * b));
}

static inline float f1(float theta) {
	float b = 1.f / (theta + 0.02f);
	return 20.91f * b * b * b;
}

static inline float f2(float theta) {
	float b = 1.f / (theta + 0.02f);
	return 72.37f * b * b;
}

static inline float f3(float theta, float lambda) {
	float b = theta - 3.f * (lambda / 568.f);
	return 436.9f * (568.f / lambda) * math::exp(-(b * b));
}

void Glare3::init(const scene::camera::Camera& camera, thread::Pool& pool) {
	const auto dim = camera.sensor_dimensions();
	dimensions_ = dim;
	high_pass_ = memory::allocate_aligned<float3>(dim[0] * dim[1]);

	// This seems a bit arbitrary
	const float solid_angle = 0.5f * math::radians_to_degrees(camera.pixel_solid_angle());

	kernel_dimensions_ = 2 * dim;
	int32_t kernel_size = kernel_dimensions_[0] * kernel_dimensions_[1];
	kernel_ = memory::allocate_aligned<float3>(kernel_size);

	const spectrum::Interpolated CIE_X(spectrum::CIE_Wavelengths_360_830_1nm, spectrum::CIE_X_360_830_1nm, spectrum::CIE_XYZ_Num);
	const spectrum::Interpolated CIE_Y(spectrum::CIE_Wavelengths_360_830_1nm, spectrum::CIE_Y_360_830_1nm, spectrum::CIE_XYZ_Num);
	const spectrum::Interpolated CIE_Z(spectrum::CIE_Wavelengths_360_830_1nm, spectrum::CIE_Z_360_830_1nm, spectrum::CIE_XYZ_Num);

	constexpr float wl_start = 400.f;
	constexpr float wl_end = 700.f;
	constexpr int32_t wl_num_samples = 64;
	constexpr float wl_step = (wl_end - wl_start) / static_cast<float>(wl_num_samples);
	const	  float wl_norm = 1.f / CIE_Y.integrate(wl_start, wl_end);

	struct F {
		float  a;
		float  b;
		float  c;
		float3 d;
	};

	std::vector<F> f(kernel_size);

	struct Init {
		float  a_sum = 0.f;
		float  b_sum = 0.f;
		float  c_sum = 0.f;
		float3 d_sum = float3(0.f);
	};

	std::vector<Init> inits(pool.num_threads());

	pool.run_range([this, dim, solid_angle,  wl_start, wl_step, wl_norm,
				   wl_num_samples, &CIE_X, &CIE_Y, &CIE_Z, &f, &inits]
				   (uint32_t id, int32_t begin, int32_t end) {
		Init& init = inits[id];

		for (int32_t y = begin; y < end; ++y) {
			for (int32_t x = 0; x < kernel_dimensions_[0]; ++x) {
				int2 p(-dim[0] + x, -dim[1] + y);

				float theta = math::length(float2(p)) * solid_angle;

				float a = f0(theta);
				float b = f1(theta);
				float c = f2(theta);

				init.a_sum += a;
				init.b_sum += b;
				init.c_sum += c;

				float3 d(0.f);

				if (Adaption::Photopic != adaption_) {
					float3 xyz(0.f);
					for (int32_t k = 0; k < wl_num_samples; ++k) {
						float lambda = wl_start + static_cast<float>(k) * wl_step;
						float val = wl_norm * f3(theta , lambda);
						xyz[0] += CIE_X.evaluate(lambda) * val;
						xyz[1] += CIE_Y.evaluate(lambda) * val;
						xyz[2] += CIE_Z.evaluate(lambda) * val;
					}

					d = math::max(spectrum::XYZ_to_linear_RGB(xyz), float3(0.f));

					init.d_sum += d;
				}

				int32_t i = y * kernel_dimensions_[0] + x;
				f[i] = F{ a, b, c, d };
			}
		}
	}, 0, kernel_dimensions_[1]);

	float a_sum = 0.f;
	float b_sum = 0.f;
	float c_sum = 0.f;
	float3 d_sum(0.f);

	for (auto i : inits) {
		a_sum += i.a_sum;
		b_sum += i.b_sum;
		c_sum += i.c_sum;
		d_sum += i.d_sum;
	}

	float scale[4];

	switch (adaption_) {
	case Adaption::Scotopic:
		scale[0] = 0.282f;
		scale[1] = 0.478f;
		scale[2] = 0.207f;
		scale[3] = 0.033f;
		break;
	default:
	case Adaption::Mesopic:
		scale[0] = 0.368f;
		scale[1] = 0.478f;
		scale[2] = 0.138f;
		scale[3] = 0.016f;
		break;
	case Adaption::Photopic:
		scale[0] = 0.383f;
		scale[1] = 0.478f;
		scale[2] = 0.138f;
		scale[3] = 0.f;
		break;
	}

	const float a_n = scale[0] / a_sum;
	const float b_n = scale[1] / b_sum;
	const float c_n = scale[2] / c_sum;

	if (Adaption::Photopic == adaption_) {
		for (int32_t i = 0, len = kernel_size; i < len; ++i) {
			kernel_[i] = float3(a_n * f[i].a + b_n * f[i].b + c_n * f[i].c);
		}
	} else {
		const float3 d_n = float3(scale[3]) / d_sum;

		for (int32_t i = 0, len = kernel_size; i < len; ++i) {
			kernel_[i] = float3(a_n * f[i].a + b_n * f[i].b + c_n * f[i].c) + d_n * f[i].d;
		}
	}

	// Gaussian blur
	const float radius = 16.f; //static_cast<float>(std::max(dim[0], dim[1])) * 0.00125f;
	gauss_width_ = 2 * std::max(static_cast<int32_t>(radius + 0.5f), 1) + 1;

	gauss_kernel_ = memory::allocate_aligned<K>(gauss_width_);

	const float fr = radius + 0.5f;
	const math::filter::Gaussian_functor gauss(static_cast<float>(fr * fr), /*radius * 0.5f*/0.15f);

	const int32_t ir = static_cast<int32_t>(radius);

	for (int32_t x = 0, len = gauss_width_; x < len; ++x) {
		const int32_t o = -ir + x;

		const float fo = static_cast<float>(o);
		const float w = gauss(fo * fo);

		gauss_kernel_[x] = K{o, w};
	}
}

size_t Glare3::num_bytes() const {
	return sizeof(*this) +
			(dimensions_[0] * dimensions_[1]) * sizeof(float3) +
			(kernel_dimensions_[0] * kernel_dimensions_[1]) * sizeof(float3);
}

void Glare3::apply(uint32_t id, uint32_t pass, int32_t begin, int32_t end,
				   const image::Float4& source, image::Float4& destination) {
	if (0 == pass) {
		const float threshold = threshold_;

		for (int32_t i = begin; i < end; ++i) {
			const float3 color = source.at(i).xyz();
			const float l = spectrum::luminance(color);

			if (l > threshold) {
				high_pass_[i] = color;
			} else {
				high_pass_[i] = float3(0.f);
			}
		}
	} else if (1 == pass) {
		const auto d = destination.description().dimensions.xy();

		const float fdm0 = static_cast<float>(d[0] - 1);
		const float fdm1 = static_cast<float>(d[1] - 1);

		const int32_t num_samples = 4096;//std::max(d[0], d[1]) * 2;

		const float weight = static_cast<float>(d[0] * d[1]) / static_cast<float>(num_samples);

		const float intensity = weight * intensity_;
		// const Vector intensity = simd::set_float4(intensity_);

		const int32_t kd0 = kernel_dimensions_[0];

		rnd::Generator rng(0, id);

		for (int32_t i = begin; i < end; ++i) {
			const int2 c = destination.coordinates_2(i);
			const int2 kb = d - c;

			const float2 center = (float2(c) + float2(0.5f)) / float2(d);

			const uint32_t r = rng.random_uint();

			float3 glare(0.f);
			for (int32_t j = 0; j < num_samples; ++j) {
//				const float r0 = rng.random_float();
//				const float r1 = rng.random_float();

//				const int32_t sx = static_cast<int32_t>(r0 * fdm0);
//				const int32_t sy = static_cast<int32_t>(r1 * fdm1);

				float2 uv = math::hammersley(j, num_samples, r);

				const float2 d = uv - center;
				const float scale = 2.f * std::max(std::abs(d[0]), std::abs(d[1]));

				uv = scale * d + center;

				const int32_t sx = static_cast<int32_t>(uv[0] * fdm0);
				const int32_t sy = static_cast<int32_t>(uv[1] * fdm1);

				const int32_t si = sy * d[0] + sx;

				const int2 kc = kb + int2(sx, sy);

				const int32_t ki = kc[1] * kd0 + kc[0];

				const float3 k = kernel_[ki];

				glare += k * high_pass_[si];
			}

			destination.at(i) = float4(intensity * glare);

		//	float4 s = source.load(i);

		//	destination.at(i) = float4(s.xyz() + intensity * glare, s[3]);

/*
			Vector glare = simd::Zero;
			for (int32_t ky = kb[1], krow = kb[1] * kd0; ky < ke[1]; ++ky, krow += kd0) {
				int32_t si = (cd1 + ky) * d[0];
				for (int32_t ki = kb[0] + krow, kl = ke[0] + krow; ki < kl; ++ki, ++si) {
					const Vector k = simd::load_float4(kernel_[ki].v);
					const Vector h = simd::load_float4(high_pass_[si].v);

					glare = math::add(glare, math::mul(k, h));
				}
			}

			glare = math::mul(glare, intensity);

			Vector s = simd::load_float4(reinterpret_cast<float*>(source.address(i)));
			s = math::add(s, glare);
			simd::store_float4(reinterpret_cast<float*>(destination.address(i)), s);
*/
		}
	} else if (2 == pass) {
		// vertical

		const auto d = destination.description().dimensions.xy();

		for (int32_t i = begin; i < end; ++i) {
			const int2 c = destination.coordinates_2(i);
			float3 accum(0.f);
			float weight_sum = 0.f;
			for (int32_t j = 0, len = gauss_width_; j < len; ++j) {
				const auto k = gauss_kernel_[j];
				int32_t kx = c[0] + k.o;

				if (kx >= 0 && kx < d[0]) {
					const int32_t si = c[1] * d[0] + kx;
					const float3 v = destination.at(si).xyz();
					accum += k.w * v;
					weight_sum += k.w;
				}
			}

			const int32_t di = c[1] * d[0] + c[0];
			high_pass_[di] = accum / weight_sum;
		}
	} else if (3 == pass) {
		// horizontal

		const auto d = destination.description().dimensions.xy();

		for (int32_t i = begin; i < end; ++i) {
			const int2 c = destination.coordinates_2(i);
			float3 accum(0.f);
			float weight_sum = 0.f;
			for (int32_t j = 0, len = gauss_width_; j < len; ++j) {
				const auto k = gauss_kernel_[j];
				int32_t ky = c[1] + k.o;

				if (ky >= 0 && ky < d[1]) {
					const int32_t si = ky * d[0] + c[0];
					const float3 v = high_pass_[si];
					accum += k.w * v;
					weight_sum += k.w;
				}
			}

			float4 s = source.load(i);

			destination.at(i) = float4(s.xyz() + accum / weight_sum, s[3]);
		}
	}
}

}}
