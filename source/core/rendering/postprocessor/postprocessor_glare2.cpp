#include "postprocessor_glare2.hpp"
#include "image/typed_image.inl"
#include "base/memory/align.hpp"
#include "base/math/exp.hpp"
#include "base/math/vector.inl"
#include "base/math/vector4.inl"
#include "base/math/filter/gaussian.hpp"
#include "base/math/fourier/dft.hpp"
#include "base/spectrum/interpolated.hpp"
#include "base/spectrum/rgb.hpp"
#include "base/spectrum/xyz.hpp"
#include "base/thread/thread_pool.hpp"
#include <vector>

// #include "image/encoding/png/png_writer.hpp"

namespace rendering { namespace postprocessor {

Glare2::Glare2(Adaption adaption, float threshold, float intensity) :
	Postprocessor(2),
	adaption_(adaption),
	threshold_(threshold), intensity_(intensity),
	kernel_dft_r_(nullptr),
	kernel_dft_g_(nullptr),
	kernel_dft_b_(nullptr),
	high_pass_r_(nullptr),
	high_pass_g_(nullptr),
	high_pass_b_(nullptr),
	high_pass_dft_r_(nullptr),
	high_pass_dft_g_(nullptr),
	high_pass_dft_b_(nullptr),
	temp_(nullptr) {}

Glare2::~Glare2() {
	memory::free_aligned(temp_);
	memory::free_aligned(high_pass_dft_b_);
	memory::free_aligned(high_pass_dft_g_);
	memory::free_aligned(high_pass_dft_r_);
	memory::free_aligned(high_pass_b_);
	memory::free_aligned(high_pass_g_);
	memory::free_aligned(high_pass_r_);
	memory::free_aligned(kernel_dft_b_);
	memory::free_aligned(kernel_dft_g_);
	memory::free_aligned(kernel_dft_r_);
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

void Glare2::init(const scene::camera::Camera& camera, thread::Pool& pool) {
	// This seems a bit arbitrary
	const float solid_angle = 0.5f * math::radians_to_degrees(camera.pixel_solid_angle());

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

	const auto dim = camera.sensor_dimensions();
	kernel_dimensions_ = 2 * dim;
	const int32_t kernel_size = kernel_dimensions_[0] * kernel_dimensions_[1];

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
				const int2 p(-dim[0] + x, -dim[1] + y);

				const float theta = math::length(float2(p)) * solid_angle;

				const float a = f0(theta);
				const float b = f1(theta);
				const float c = f2(theta);

				init.a_sum += a;
				init.b_sum += b;
				init.c_sum += c;

				float3 d(0.f);

				if (Adaption::Photopic != adaption_) {
					float3 xyz(0.f);
					for (int32_t k = 0; k < wl_num_samples; ++k) {
						const float lambda = wl_start + static_cast<float>(k) * wl_step;
						const float val = wl_norm * f3(theta , lambda);
						xyz[0] += CIE_X.evaluate(lambda) * val;
						xyz[1] += CIE_Y.evaluate(lambda) * val;
						xyz[2] += CIE_Z.evaluate(lambda) * val;
					}

					d = math::max(spectrum::XYZ_to_linear_RGB(xyz), float3(0.f));

					init.d_sum += d;
				}

				const int32_t i = y * kernel_dimensions_[0] + x;
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

	float* kernel_r = memory::allocate_aligned<float>(kernel_size);
	float* kernel_g = memory::allocate_aligned<float>(kernel_size);
	float* kernel_b = memory::allocate_aligned<float>(kernel_size);

	if (Adaption::Photopic == adaption_) {
		for (int32_t i = 0; i < kernel_size; ++i) {
			const float k = a_n * f[i].a + b_n * f[i].b + c_n * f[i].c;

			kernel_r[i] = k;
			kernel_g[i] = k;
			kernel_b[i] = k;
		}
	} else {
		const float3 d_n = scale[3] / d_sum;

		for (int32_t i = 0; i < kernel_size; ++i) {
			const float3 k((a_n * f[i].a + b_n * f[i].b + c_n * f[i].c) + d_n * f[i].d);

			kernel_r[i] = k[0];
			kernel_g[i] = k[1];
			kernel_b[i] = k[2];
		}
	}

	const int32_t kernel_dft_size = math::dft_size(kernel_dimensions_[0]) * kernel_dimensions_[1];

	kernel_dft_r_ = memory::allocate_aligned<float2>(kernel_dft_size);
	kernel_dft_g_ = memory::allocate_aligned<float2>(kernel_dft_size);
	kernel_dft_b_ = memory::allocate_aligned<float2>(kernel_dft_size);

	temp_ = memory::allocate_aligned<float2>(kernel_dft_size);

	math::dft_2d(kernel_dft_r_, kernel_r, temp_,
				 kernel_dimensions_[0], kernel_dimensions_[1], pool);

	math::dft_2d(kernel_dft_g_, kernel_g, temp_,
				 kernel_dimensions_[0], kernel_dimensions_[1], pool);

	math::dft_2d(kernel_dft_b_, kernel_b, temp_,
				 kernel_dimensions_[0], kernel_dimensions_[1], pool);

	memory::free_aligned(kernel_b);
	memory::free_aligned(kernel_g);
	memory::free_aligned(kernel_r);

	high_pass_r_ = memory::allocate_aligned<float>(kernel_size);
	high_pass_g_ = memory::allocate_aligned<float>(kernel_size);
	high_pass_b_ = memory::allocate_aligned<float>(kernel_size);

	high_pass_dft_r_ = memory::allocate_aligned<float2>(kernel_dft_size);
	high_pass_dft_g_ = memory::allocate_aligned<float2>(kernel_dft_size);
	high_pass_dft_b_ = memory::allocate_aligned<float2>(kernel_dft_size);
}

size_t Glare2::num_bytes() const {
	const size_t kernel_size = static_cast<size_t>(kernel_dimensions_[0] * kernel_dimensions_[1]);

	const size_t kernel_dft_size = static_cast<size_t>(math::dft_size(kernel_dimensions_[0]) *
													   kernel_dimensions_[1]);

	return sizeof(*this) +
		kernel_size * sizeof(float) * 3  +
		kernel_dft_size * sizeof(float2) * 7;
}

static inline float2 mul_complex(float2 a, float2 b, float scale) {
	return scale * float2(a[0] * b[0] - a[1] * b[1], a[0] * b[1] + a[1] * b[0]);
}

void Glare2::pre_apply(const image::Float_4& source, image::Float_4& destination,
					   thread::Pool& pool) {
	const auto dim = kernel_dimensions_;

	pool.run_range([this, dim, &source]
		(uint32_t /*id*/, int32_t begin, int32_t end) {
			const float threshold = threshold_;

			const int2 offset = dim / 4;

			const int2 source_dim = source.dimensions2();

			for (int32_t y = begin, i = begin * dim[0]; y < end; ++y) {
				for (int32_t x = 0; x < dim[0]; ++x, ++i) {
					float3 out(0.f);

					int2 sc = int2(x, y) - offset;
					if (sc[0] > 0 && sc[1] > 0
					&&  sc[0] < source_dim[0] && sc[1] < source_dim[1]) {
						float3 color = source.at(sc[0], sc[1]).xyz();

						float l = spectrum::luminance(color);

						if (l > threshold) {
							out = color;
						}
					}

					high_pass_r_[i] = out[0];
					high_pass_g_[i] = out[1];
					high_pass_b_[i] = out[2];
				}
			}
		}, 0, dim[1]);

//	image::encoding::png::Writer::write("high_pass_r.png", high_pass_r_, dim, 16.f);
//	image::encoding::png::Writer::write("high_pass_g.png", high_pass_g_, dim, 16.f);
//	image::encoding::png::Writer::write("high_pass_b.png", high_pass_b_, dim, 16.f);

	const int32_t kernel_dft_size = math::dft_size(kernel_dimensions_[0]) * kernel_dimensions_[1];

	math::dft_2d(high_pass_dft_r_, high_pass_r_, temp_, dim[0], dim[1], pool);
	math::dft_2d(high_pass_dft_g_, high_pass_g_, temp_, dim[0], dim[1], pool);
	math::dft_2d(high_pass_dft_b_, high_pass_b_, temp_, dim[0], dim[1], pool);

	pool.run_range([this, dim]
		(uint32_t /*id*/, int32_t begin, int32_t end) {
			const float scale = 1.f / (dim[0] * dim[1]);
			for (int32_t i = begin; i < end; ++i) {
				high_pass_dft_r_[i] = mul_complex(high_pass_dft_r_[i], kernel_dft_r_[i], scale);
				high_pass_dft_g_[i] = mul_complex(high_pass_dft_g_[i], kernel_dft_g_[i], scale);
				high_pass_dft_b_[i] = mul_complex(high_pass_dft_b_[i], kernel_dft_b_[i], scale);

//				high_pass_dft_r_[i] *= scale;
//				high_pass_dft_g_[i] *= scale;
//				high_pass_dft_b_[i] *= scale;
			}
		}, 0, kernel_dft_size);


//	int2 kernel_dft_dimensions(math::dft_size(kernel_dimensions_[0]), kernel_dimensions_[1]);
//	image::encoding::png::Writer::write("high_pass_dft_r.png", high_pass_dft_r_,
//										kernel_dft_dimensions, 16.f);

	math::idft_2d(high_pass_r_, high_pass_dft_r_, temp_, dim[0], dim[1], pool);
	math::idft_2d(high_pass_g_, high_pass_dft_g_, temp_, dim[0], dim[1], pool);
	math::idft_2d(high_pass_b_, high_pass_dft_b_, temp_, dim[0], dim[1], pool);

	const int2 offset = dim / 4;

	pool.run_range([this, dim, offset, &source, &destination]
		(uint32_t /*id*/, int32_t begin, int32_t end) {
			const int2 source_dim = source.dimensions2();

			const float intensity = intensity_;

			for (int32_t y = begin; y < end; ++y) {
				for (int32_t x = offset[0], width = offset[0] + source_dim[0]; x < width; ++x) {
					const int2 sc = int2(x, y) - offset;

					const int32_t iy = (y + source_dim[1]) % dim[1];
					const int32_t ix = (x + source_dim[0]) % dim[0];
					const int32_t i = iy * dim[0] + ix;

					const auto& s = source.at(sc[0], sc[1]);
					float3 glare(high_pass_r_[i], high_pass_g_[i], high_pass_b_[i]);
					glare = math::max(glare, float3(0.f));

					destination.store(sc[0], sc[1], float4(s.xyz() + intensity * glare, s[3]));
				}
			}
		}, offset[1], offset[1] + source.dimensions2()[1]);


//	image::encoding::png::Writer::write("high_pass_ro.png", high_pass_r_, dim, 128.f);
}

void Glare2::apply(int32_t /*begin*/, int32_t /*end*/, uint32_t /*pass*/,
				   const image::Float_4& /*source*/, image::Float_4& /*destination*/) {
	return;
}

}}
