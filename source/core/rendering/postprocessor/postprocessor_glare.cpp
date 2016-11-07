#include "postprocessor_glare.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"
#include "base/math/filter/gaussian.hpp"
#include "base/spectrum/interpolated.hpp"
#include "base/spectrum/rgb.inl"
#include "base/spectrum/xyz.inl"
#include "base/thread/thread_pool.hpp"

#include "base/math/print.hpp"
#include <iostream>

namespace rendering { namespace postprocessor {

Glare::Glare(Adaption adaption, float threshold, float intensity) :
	Postprocessor(2),
	adaption_(adaption),
	threshold_(threshold), intensity_(intensity) {}

float f0(float theta) {
	float b = theta / 0.02f;
	return 2.61f * 10e6f * std::exp(-(b * b));
}

float f1(float theta) {
	float b = 1.f / (theta + 0.02f);
	return 20.91f * b * b * b;
}

float f2(float theta) {
	float b = 1.f / (theta + 0.02f);
	return 72.37f * b * b;
}

float f3(float theta, float lambda) {
	float b = theta - 3.f * (lambda / 568.f);
	return 436.9f * (568.f / lambda) * std::exp(-(b * b));
}

void Glare::init(const scene::camera::Camera& camera, thread::Pool& pool) {
	auto d = camera.sensor_dimensions();

	high_pass_.resize(d.x * d.y);

	// This seems a bit arbitrary
	float solid_angle = 0.5f * math::radians_to_degrees(camera.pixel_solid_angle());

	kernel_dimensions_ = 2 * d;
	kernel_.resize(kernel_dimensions_.x * kernel_dimensions_.y);

	const spectrum::Interpolated CIE_X(spectrum::CIE_Wavelengths_360_830_1nm, spectrum::CIE_X_360_830_1nm, spectrum::CIE_XYZ_Num);
	const spectrum::Interpolated CIE_Y(spectrum::CIE_Wavelengths_360_830_1nm, spectrum::CIE_Y_360_830_1nm, spectrum::CIE_XYZ_Num);
	const spectrum::Interpolated CIE_Z(spectrum::CIE_Wavelengths_360_830_1nm, spectrum::CIE_Z_360_830_1nm, spectrum::CIE_XYZ_Num);

	constexpr float wl_start = 400.f;
	constexpr float wl_end = 700.f;
	constexpr int32_t wl_num_samples = 64;
	float wl_norm = 1.f / CIE_Y.integrate(wl_start, wl_end);
	constexpr float wl_step = (wl_end - wl_start) / static_cast<float>(wl_num_samples);

	float a_sum = 0.f;
	float b_sum = 0.f;
	float c_sum = 0.f;
	float3 d_sum(0.f);

	struct F {
		float  a;
		float  b;
		float  c;
		float3 d;
	};

	std::vector<F> f(kernel_.size());

	Init* inits = new Init[pool.num_threads()];

	pool.run([inits](uint32_t id) { inits[id].init(id); });

	for (int32_t y = 0; y < kernel_dimensions_.y; ++y) {
		for (int32_t x = 0; x < kernel_dimensions_.x; ++x) {
			int2 p(-d.x + x, -d.y + y);

			float theta = math::length(float2(p)) * solid_angle;

			float a = f0(theta);
			float b = f1(theta);
			float c = f2(theta);

			a_sum += a;
			b_sum += b;
			c_sum += c;

			float3 d(0.f);

			if (Adaption::Photopic != adaption_) {
				float3 xyz(0.f);
				for (int32_t k = 0; k < wl_num_samples; ++k) {
					float lambda = wl_start + static_cast<float>(k) * wl_step;
					float val = wl_norm * f3(theta , lambda);
					xyz.x += CIE_X.evaluate(lambda) * val;
					xyz.y += CIE_Y.evaluate(lambda) * val;
					xyz.z += CIE_Z.evaluate(lambda) * val;
				}

				d = math::max(spectrum::XYZ_to_linear_RGB(xyz), float3(0.f));

				d_sum += d;
			}

			int32_t i = y * kernel_dimensions_.x + x;
			f[i] = F{ a, b, c, d };
		}
	}

	float scale[4];

	switch (adaption_) {
	case Adaption::Scotopic:
		scale[0] = 0.282f;
		scale[1] = 0.478f;
		scale[2] = 0.207f;
		scale[3] = 0.033f;
		break;
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

	float  a_n = scale[0] / a_sum;
	float  b_n = scale[1] / b_sum;
	float  c_n = scale[2] / c_sum;

	if (Adaption::Photopic == adaption_) {
		for (size_t i = 0, len = kernel_.size(); i < len; ++i) {
			kernel_[i] = float3(a_n * f[i].a + b_n * f[i].b + c_n * f[i].c);
		}
	} else {
		float3 d_n = scale[3] / d_sum;

		for (size_t i = 0, len = kernel_.size(); i < len; ++i) {
			kernel_[i] = float3(a_n * f[i].a + b_n * f[i].b + c_n * f[i].c) + d_n * f[i].d;
		}
	}
}

size_t Glare::num_bytes() const {
	return sizeof(*this) +
			high_pass_.size() * sizeof(float3) +
			kernel_.size() * sizeof(float3);
}

void Glare::Init::init(uint32_t id) {
	std::cout << "init(" << id << ")" << std::endl;

	a_sum = 0.f;
	b_sum = 0.f;
	c_sum = 0.f;
	d_sum = float3(0.f);
}

void Glare::apply(int32_t begin, int32_t end, uint32_t pass,
				  const image::Image_float_4& source,
				  image::Image_float_4& destination) {
	if (0 == pass) {
		float threshold = threshold_;

		for (int32_t i = begin; i < end; ++i) {
			float3 color = source.at(i).xyz;

			float l = spectrum::luminance(color);

			if (l > threshold) {
				high_pass_[i] = color;
			} else {
				high_pass_[i] = float3(0.f);
			}
		}
	} else {
		float intensity = intensity_;

		auto d = destination.description().dimensions.xy;

		int2 hkd = kernel_dimensions_ / 2;

		for (int32_t i = begin; i < end; ++i) {
			int2 c = destination.coordinates_2(i);

			int2 kb = hkd - c;
			int2 ke = kb + d;

			float3 glare(0.f);
			for (int32_t ky = kb.y; ky < ke.y; ++ky) {
				int32_t krow = ky * kernel_dimensions_.x;
				int32_t srow = (c.y - d.y + ky) * d.x;
				for (int32_t kx = kb.x; kx < ke.x; ++kx) {
					float3 k = kernel_[krow + kx];

					int32_t sx = c.x - d.x + kx;
					glare += k * high_pass_[srow + sx];
				}
			}

			float4 s = source.load(i);

			destination.at(i) = float4(s.xyz + intensity * glare, s.w);
		}
	}
}

}}
