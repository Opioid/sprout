#include "starburst.hpp"
#include "aperture.hpp"
#include "core/image/typed_image.inl"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/filter/image_gaussian.inl"
#include "core/image/procedural/image_renderer.hpp"
#include "base/math/vector.inl"
#include "base/math/fourier/dft.hpp"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/rgb.inl"
#include "base/spectrum/xyz.inl"
#include "base/thread/thread_pool.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace procedural { namespace starburst {

static constexpr uint32_t Num_bands = 64;

using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

void centered_magnitude(float* result, const float2* source, size_t width, size_t height);

void starburst(Spectrum* result, const float* source, int32_t bin, float d, int32_t resolution);

void write_signal(const std::string& name, const image::Image_float_1& signal);

void create(thread::Pool& pool) {
	std::cout << "Starburst experiment" << std::endl;

	Spectrum::init(380.f, 720.f);

	int32_t resolution = 16;

	int2 dimensions(resolution, resolution);

	image::Image_float_1 signal(image::Image::Description(image::Image::Type::Float_1, dimensions));

	std::vector<float2> signal_f(resolution * math::dft_size(resolution));

	image::Image_float_3 float_image_a(image::Image::Description(image::Image::Type::Float_3,
																 dimensions));

	image::Image_float_3 float_image_b(image::Image::Description(image::Image::Type::Float_3,
																 dimensions));

	image::Image_byte_3 byte_image(image::Image::Description(image::Image::Type::Byte_3,
															 dimensions));

	Spectrum* spectral_data = new Spectrum[resolution * resolution];

	float radius = static_cast<float>(resolution) * 0.00390625f;
	image::filter::Gaussian<math::packed_float3> gaussian(radius, radius * 0.0005f);


	image::procedural::Mini_renderer renderer(signal);

	renderer.set_brush(0.f);
	renderer.clear();

	renderer.set_brush(0.4f);
	renderer.draw_circle(float2(0.4f, 0.3f), 0.2f);

	Aperture aperture(5);

	float fr = static_cast<float>(resolution);

	for (int32_t y = 0; y < resolution; ++y) {
		for (int32_t x = 0; x < resolution; ++x) {
			float2 p(-1.f + 2.f * ((static_cast<float>(x) + 0.5f) / fr),
					  1.f - 2.f * ((static_cast<float>(y) + 0.5f) / fr));


		//	std::cout << p << std::endl;

		//	float s = signal.load(x, y);

			float a = aperture.evaluate(p, fr);



			signal.store(x, y, a);
		}
	}



	write_signal("signal.png", signal);

	math::dft_2d(signal_f.data(), signal.data(), resolution, resolution, pool);

	centered_magnitude(signal.data(), signal_f.data(), resolution, resolution);

	pool.run_range([spectral_data, &signal, resolution](int32_t begin, int32_t end) {
		float d = 1.f / 720.f;
		for (int32_t bin = begin; bin < end; ++bin) {
			starburst(spectral_data, signal.data(), bin, d, resolution);
		}
	}, 0, Spectrum::num_bands());

	for (int32_t i = 0, len = resolution * resolution; i < len; ++i) {
		auto& s = spectral_data[i];
		float3 linear_rgb = spectrum::XYZ_to_linear_RGB(s.normalized_XYZ());
		float_image_a.store(i, math::packed_float3(linear_rgb));
	}

	gaussian.apply(float_image_a, float_image_b);

	for (int32_t i = 0, len = resolution * resolution; i < len; ++i) {
		float3 linear_rgb = float3(float_image_b.load(i));
		byte3 srgb = spectrum::float_to_unorm(spectrum::linear_RGB_to_sRGB(linear_rgb));
		byte_image.store(i, srgb);
	}

	delete [] spectral_data;

	image::encoding::png::Writer::write("starburst.png", byte_image);
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
			float2 p(-1.f + 2.f * ((static_cast<float>(x) + 0.5f) / fr),
					  1.f - 2.f * ((static_cast<float>(y) + 0.5f) / fr));

			float2 q = p * (wavelength * d);

			q.x =  0.5f * (q.x + 1.f);
			q.y = -0.5f * (q.y - 1.f);

			int32_t sx = static_cast<int32_t>(q.x * fr - 0.5f);
			int32_t sy = static_cast<int32_t>(q.y * fr - 0.5f);

			int32_t i = sy * resolution + sx;
			float mag = source[i];

			float v = std::max(1.f - math::dot(p, p), 0.f);

			float r = v * scale * (mag * mag);

			int32_t o = y * resolution + x;
			result[o].set_bin(bin, r);
		}
	}
}

void write_signal(const std::string& name, const image::Image_float_1& signal) {
	auto d = signal.description().dimensions;

	image::Image_byte_1 image(image::Image::Description(image::Image::Type::Byte_1, d));

	for (int32_t i = 0, len = d.x * d.y; i < len; ++i) {
		float s = signal.load(i);
		uint8_t b = spectrum::float_to_unorm(s);
		image.store(i, b);
	}

	image::encoding::png::Writer::write(name, image);
}

}}
