#include "starburst.hpp"
#include "aperture.hpp"
#include "core/image/typed_image.inl"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/filter/image_gaussian.inl"
#include "core/image/procedural/image_renderer.hpp"
#include "base/math/vector.inl"
#include "base/math/fourier/dft.hpp"
#include "base/math/random/generator.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/rgb.inl"
#include "base/spectrum/xyz.inl"
#include "base/thread/thread_pool.hpp"

#include <iostream>
#include "base/math/print.hpp"

namespace procedural { namespace starburst {

static constexpr uint32_t Num_bands = 64;

using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

void dirt(image::Image_float_1& signal);

void centered_magnitude(float* result, const float2* source, size_t width, size_t height);

void starburst(Spectrum* result, const float* source, int32_t bin, int32_t resolution);

void write_signal(const std::string& name, const image::Image_float_1& signal);

void create(thread::Pool& pool) {
	std::cout << "Starburst experiment" << std::endl;

	Spectrum::init(380.f, 720.f);

	int32_t resolution = 512;

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

	signal.clear(1.f);
//	dirt(signal);

	Aperture aperture(8);

	float fr = static_cast<float>(resolution);

	for (int32_t y = 0; y < resolution; ++y) {
		for (int32_t x = 0; x < resolution; ++x) {
			float2 p(-1.f + 2.f * ((static_cast<float>(x) + 0.5f) / fr),
					  1.f - 2.f * ((static_cast<float>(y) + 0.5f) / fr));
			float s = signal.load(x, y);
			float a = aperture.evaluate(p, fr);

			signal.store(x, y, s * a);
		}
	}



	write_signal("signal.png", signal);

	math::dft_2d(signal_f.data(), signal.data(), resolution, resolution, pool);

	centered_magnitude(signal.data(), signal_f.data(), resolution, resolution);

	write_signal("signal_after.png", signal);

	pool.run_range([spectral_data, &signal, resolution](int32_t begin, int32_t end) {
		for (int32_t bin = begin; bin < end; ++bin) {
			starburst(spectral_data, signal.data(), bin, resolution);
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

void dirt(image::Image_float_1& signal) {
	image::procedural::Mini_renderer renderer(signal);

//	renderer.set_brush(1.f);
//	renderer.clear();


	renderer.set_brush(0.9f);
	renderer.draw_circle(float2(0.5f), 0.4f);
	renderer.set_brush(1.f);
	renderer.draw_circle(float2(0.5f), 0.38f);

	renderer.set_brush(0.9f);
	renderer.draw_circle(float2(0.5f), 0.36f);
	renderer.set_brush(1.f);
	renderer.draw_circle(float2(0.5f), 0.34f);

	renderer.set_brush(0.9f);
	renderer.draw_circle(float2(0.5f), 0.32f);
	renderer.set_brush(1.f);
	renderer.draw_circle(float2(0.5f), 0.3f);

	math::random::Generator rng(0, 1, 2, 3);

	uint32_t num_dirt = 128 * 1024;

	for (uint32_t i = 0; i < num_dirt; ++i) {
		float2 p = math::hammersley(i, num_dirt);

		float rc = rng.random_float();
		renderer.set_brush(0.7f + 0.3f * rc);

		float rs = rng.random_float();
		renderer.draw_circle(p, 0.0025f + rs * 0.0025f);
	}
}

void centered_magnitude(float* result, const float2* source, size_t width, size_t height) {
	size_t row_size = math::dft_size(width);

	for (size_t y = 0; y < height; ++y) {
		size_t ro = y;

		if (y < height / 2) {
			ro = y + height / 2;
		} else {
			ro = y - height / 2;
		}


//		size_t o = y * row_size;
//		float mag = /*0.001f **/ math::length(source[o]);

//		result[ro * width + row_size - 1] = mag;


		for (size_t x = 0, len = row_size - 1; x < len; ++x) {
			size_t o = y * row_size + x;
			float mag = /*0.001f **/ math::length(source[o]);

	//		size_t a = ro * width + x + len;
	//		std::cout << a << std::endl;

			result[ro * width + x + len] = mag;
		}

		for (size_t x = 0, len = row_size - 1; x < len; ++x) {
			size_t o = y * row_size + x;
			float mag = /*0.001f **/ math::length(source[o]);

	//		size_t a = ro * width - x + len - 1;
	//		std::cout << a << std::endl;

			result[ro * width - x + len - 1] = mag;
		}

	//	std::cout << "row end" << std::endl;
	}
}

void starburst(Spectrum* result, const float* source, int32_t bin, int32_t resolution) {
	float scale = 4.f / (resolution * resolution);

	float fr = static_cast<float>(resolution);

	float wavelength = Spectrum::wavelength_center(bin);

	float d = wavelength * (1.f / Spectrum::wavelength_center(Spectrum::num_bands() - 1));

	for (int32_t y = 0; y < resolution; ++y) {
		for (int32_t x = 0; x < resolution; ++x) {
			float2 p(-1.f + 2.f * ((static_cast<float>(x) + 0.5f) / fr),
					  1.f - 2.f * ((static_cast<float>(y) + 0.5f) / fr));

			float2 q = d * p;

			q.x =  0.5f * (q.x + 1.f);
			q.y = -0.5f * (q.y - 1.f);

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
