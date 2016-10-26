#include "starburst.hpp"
#include "aperture.hpp"
#include "core/image/typed_image.inl"
#include "core/image/encoding/png/png_writer.hpp"
#include "base/math/vector.inl"
#include "base/math/fourier/dft.hpp"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/rgb.hpp"
#include "base/spectrum/xyz.hpp"
#include "base/thread/thread_pool.hpp"

#include <iostream>

namespace procedural { namespace starburst {

static constexpr uint32_t Num_bands = 64;

using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

void centered_magnitude(float* result, const float2* source, size_t width, size_t height);

void starburst(Spectrum* result, const float* source, int32_t bin, float d, int32_t resolution);

void starbursts(int32_t begin, int32_t end, Spectrum* result, const float* source,
				float d, int32_t resolution);

void create(thread::Pool& pool) {
	std::cout << "Starburst experiment" << std::endl;

	Spectrum::init(380.f, 720.f);

	int32_t resolution = 512;

	int2 dimensions(resolution, resolution);

	image::Image_float_1 signal(image::Image::Description(image::Image::Type::Float_1, dimensions));

	std::vector<float2> signal_f(resolution * math::dft_size(resolution));

	image::Image_byte_3 image(image::Image::Description(image::Image::Type::Byte_3, dimensions));

	Spectrum* spectral_data = new Spectrum[resolution * resolution];

	float fr = static_cast<float>(resolution);

	Aperture aperture(5);


	for (int32_t y = 0; y < resolution; ++y) {
		for (int32_t x = 0; x < resolution; ++x) {
			float2 p(1.f - 2.f * (static_cast<float>(x) / fr),
					 1.f - 2.f * (static_cast<float>(y) / fr));

			float a = aperture.evaluate(p, fr);

			signal.store(x, y, a);

			int32_t i = y * resolution + x;
			spectral_data[i].clear(0.f);
		}
	}

	math::dft_2d(signal_f.data(), signal.data(), resolution, resolution);

	centered_magnitude(signal.data(), signal_f.data(), resolution, resolution);

	float d = 1.f / 720.f;

	pool.run_range([spectral_data, &signal, d, resolution](int32_t begin, int32_t end) {
		starbursts(begin, end, spectral_data, signal.data(), d, resolution); }, 0, Num_bands);

	for (int32_t y = 0; y < resolution; ++y) {
		for (int32_t x = 0; x < resolution; ++x) {
		//	float s = signal.load(x, y);

		//	auto rgb = spectrum::float_to_unorm(spectrum::linear_RGB_to_sRGB(float3(s)));

			int32_t i = y * resolution + x;

			auto& s = spectral_data[i];

			float3 linear_rgb = spectrum::XYZ_to_linear_RGB(s.normalized_XYZ());

			byte3 srgb = spectrum::float_to_unorm(spectrum::linear_RGB_to_sRGB(linear_rgb));

			image.at(x, y) = srgb;
		}
	}

	delete [] spectral_data;

	image::encoding::png::Writer::write("starburst.png", image);
}

void centered_magnitude(float* result, const float2* source, size_t width, size_t height) {
	size_t row_size = math::dft_size(width);

	for (size_t y = 0; y < width; ++y) {
		size_t ro;

		if (y < height / 2) {
			ro = y + height / 2;
		} else {
			ro = y - height / 2;
		}

		for (size_t x = 0; x < row_size; ++x) {
			size_t o = y * row_size + x;
			float mag = math::length(source[o]);

			result[ro * height + x + row_size - 2] = mag;
			result[ro * height - x + row_size - 1] = mag;
		}
	}
}

void starburst(Spectrum* result, const float* source, int32_t bin, float d, int32_t resolution) {
	float scale = 4.f / (resolution * resolution);

	float fr = static_cast<float>(resolution);

	float wavelength = Spectrum::wavelength_center(bin);

	for (int32_t y = 0; y < resolution; ++y) {
		for (int32_t x = 0; x < resolution; ++x) {
			float2 p(1.f - 2.f * (static_cast<float>(x) / fr),
					 1.f - 2.f * (static_cast<float>(y) / fr));

			float2 q = p * (wavelength * d);
			q = 0.5f * (q + float2(1.f));

			int32_t sx = static_cast<int32_t>(q.x * fr);
			int32_t sy = static_cast<int32_t>(q.y * fr);

			int32_t i = sy * resolution + sx;
			float mag = source[i];

			float v = std::max(1.f - math::dot(p, p), 0.f);

			float r = v * scale * (mag * mag);

			int32_t o = y * resolution + x;
			result[o].set_bin(bin, r);
		}
	}
}

void starbursts(int32_t begin, int32_t end, Spectrum* result, const float* source,
				float d, int32_t resolution) {
	for (int32_t i = begin; i < end; ++i) {
		starburst(result, source, i, d, resolution);
	}
}

}}
