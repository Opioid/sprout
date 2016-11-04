#include "postprocessor_glare.hpp"
#include "image/typed_image.inl"
#include "base/math/vector.inl"
#include "base/math/filter/gaussian.hpp"
#include "base/spectrum/interpolated.hpp"
#include "base/spectrum/rgb.inl"
#include "base/spectrum/xyz.inl"

#include "base/math/print.hpp"
#include <iostream>

namespace rendering { namespace postprocessor {

Glare::Glare(float threshold, float intensity) :
	Postprocessor(2),
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

void Glare::init(const scene::camera::Camera& camera) {
	auto d = camera.sensor_dimensions();

//	image::Image::Description description(image::Image::Type::Float_3, d);
	high_pass_.resize(d.x * d.y);

	float sensorHeight = 24.0f;
	float apertureAngle = 60.0f;
	float pixelTheta = apertureAngle/sensorHeight * math::Pi / 180.0f;

	float solid_angle = 20.f * camera.pixel_solid_angle();

	std::cout << pixelTheta << " " << solid_angle << std::endl;

	int32_t width = 2 * d.x;
	int32_t height = 2 * d.y;

	kernel_dimensions_ = int2(width, height);
	kernel_.resize(width * height);


	const spectrum::Interpolated CIE_X(spectrum::CIE_Wavelengths_360_830_1nm, spectrum::CIE_X_360_830_1nm, spectrum::CIE_XYZ_Num);
	const spectrum::Interpolated CIE_Y(spectrum::CIE_Wavelengths_360_830_1nm, spectrum::CIE_Y_360_830_1nm, spectrum::CIE_XYZ_Num);
	const spectrum::Interpolated CIE_Z(spectrum::CIE_Wavelengths_360_830_1nm, spectrum::CIE_Z_360_830_1nm, spectrum::CIE_XYZ_Num);

	float wl_start = 400.f;
	float wl_end = 700.f;
	int32_t wl_num_samples = 64;
	float wl_norm = 1.f / CIE_Y.integrate(wl_start, wl_end);
	float wl_step = (wl_end - wl_start) / static_cast<float>(wl_num_samples);


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

	std::vector<F> f;
	f.reserve(width * height);

	for (int32_t y = 0; y < height; ++y) {
		for (int32_t x = 0; x < width; ++x) {
			int2 p(-d.x + x, -d.y + y);

		//	std::cout << p << std::endl;

			float theta = math::length(float2(p)) * solid_angle;

			float a = f0(theta);
			float b = f1(theta);
			float c = f2(theta);

			float3 xyz(0.f);
			for (int32_t k = 0; k < wl_num_samples; ++k) {
				float lambda = wl_start + static_cast<float>(k) * wl_step;
				float val = wl_norm * f3(theta , lambda);
				xyz.x += CIE_X.evaluate(lambda) * val;
				xyz.y += CIE_Y.evaluate(lambda) * val;
				xyz.z += CIE_Z.evaluate(lambda) * val;
			}

			float3 d = spectrum::XYZ_to_linear_RGB(xyz);
			d = math::max(float3(0.f), d);

			a_sum += a;
			b_sum += b;
			c_sum += c;
			d_sum += d;

			f.push_back(F{a, b, c, d});
		}
	}

//	std::cout << "init done" << std::endl;

	float scotopic[4] = {0.282f, 0.478f, 0.207f, 0.033f};
	float mesopic[4] = {0.368f, 0.478f, 0.138f, 0.016f};

	float* scale = scotopic;

	float  a_n = 1.f / a_sum;
	float  b_n = 1.f / b_sum;
	float  c_n = 1.f / c_sum;
	float3 d_n = 1.f / d_sum;

	for (size_t i = 0, len = kernel_.size(); i < len; ++i) {
		kernel_[i] = float3((scale[0] * a_n * f[i].a)  +
							(scale[1] * b_n * f[i].b)  +
							(scale[2] * c_n * f[i].c)) +
							 scale[3] * d_n * f[i].d;

	//	kernel_[i].f = scale[3] * d_n * f[i].d;
	}
}

size_t Glare::num_bytes() const {
	return sizeof(*this) +
			high_pass_.size() * sizeof(float3) +
			kernel_.size() * sizeof(float3);
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
			int2 c = source.coordinates_2(i);

			int2 kb = hkd - c;

			float3 glare(0.f);

			for (int32_t ky = kb.y, key = kb.y + d.y; ky < key; ++ky) {
				for (int32_t kx = kb.x, kex = kb.x + d.x; kx < kex; ++kx) {
					int32_t ki = ky * kernel_dimensions_.x + kx;
					float3 k = kernel_[ki];

					int2 kp(-d.x + kx, -d.y + ky);
					int2 o = c + kp;
					int32_t si = o.y * d.x + o.x;

					glare += k * high_pass_[si];
				}
			}

			float4 s = source.load(i);

			destination.at(i) = float4(s.xyz + intensity * glare, s.w);
		}
	}
}

}}
