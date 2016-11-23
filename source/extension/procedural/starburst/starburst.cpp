#include "starburst.hpp"
#include "aperture.hpp"
#include "dirt.hpp"
#include "core/image/typed_image.inl"
#include "core/image/encoding/png/png_reader.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/filter/image_gaussian.inl"
#include "core/image/procedural/image_renderer.inl"
#include "core/image/texture/texture_float_2.hpp"
#include "core/image/texture/sampler/sampler_2d_linear.inl"
#include "base/math/vector.inl"
#include "base/math/fourier/dft.hpp"
#include "base/math/sampling/sample_distribution.inl"
#include "base/random/generator.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/rgb.inl"
#include "base/spectrum/xyz.inl"
#include "base/thread/thread_pool.hpp"

#include <iostream>
#include <fstream>
#include "base/math/print.hpp"

// http://onlinelibrary.wiley.com/store/10.1111/cgf.12953/asset/supinfo/cgf12953-sup-0002-S1.pdf?v=1&s=e3aaf53493a15c5111513bf1dbe1a6ee549ee804

namespace procedural { namespace starburst {

static constexpr uint32_t Num_bands = 64;

using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

void render_dirt(image::Float_1& signal);

void render_aperture(const Aperture& aperture, image::Float_1& signal);

void fdft(image::Float_2& destination, std::shared_ptr<image::Float_2> source,
		  float alpha, uint32_t mode, thread::Pool& pool);

void centered_squared_magnitude(float* result, const float2* source, size_t width, size_t height);

void squared_magnitude(float* result, const float2* source, size_t width, size_t height);

void diffraction(Spectrum* result, const float* source, uint32_t bin, int32_t resolution);

void write_signal(const std::string& name, const image::Float_1& signal);

void create(thread::Pool& pool) {
	std::cout << "Starburst experiment" << std::endl;

	using namespace image;

	Spectrum::init(380.f, 720.f);

	int32_t resolution = 512;

	int2 dimensions(resolution, resolution);

	Float_1 signal(Image::Description(Image::Type::Float_1, dimensions));

	std::vector<float2> signal_f(resolution * math::dft_size(resolution));

	Float_3 float_image_a(Image::Description(Image::Type::Float_3, dimensions));

	bool dirt = true;
	if (dirt) {
		render_dirt(signal);
	} else {
		signal.clear(1.f);
	}

//	write_signal("signal.png", signal);

	Aperture aperture(8, 0.25f, 0.015f);
//	Aperture aperture(5, 0.f, 0.f);
	render_aperture(aperture, signal);

	write_signal("signal.png", signal);

	bool near_field = false;

	if (near_field) {
		Image::Description description(Image::Type::Float_2, dimensions);
		auto signal_a = std::make_shared<Float_2>(description);
		auto signal_b = std::make_shared<Float_2>(description);


		for (int32_t i = 0, len = signal.area(); i < len; ++i) {
			signal_a->store(i, float2(signal.load(i), 0.f));
		}

		float alpha = 0.18f;

		fdft(*signal_b.get(), signal_a, alpha, 0, pool);
		fdft(*signal_a.get(), signal_b, alpha, 1, pool);
		squared_magnitude(signal.data(), signal_a->data(), resolution, resolution);

		pool.run_range([&float_image_a, &signal](int32_t begin, int32_t end) {
			for (int32_t i = begin; i < end; ++i) {
				float s = 0.6f * signal.load(i);
				float_image_a.store(i, math::packed_float3(s));
			}
		}, 0, resolution * resolution);



/*
		Float_1 signal_t(Image::Description(Image::Type::Float_1, dimensions));

		Spectrum* spectral_data = new Spectrum[resolution * resolution];


		for (uint32_t b = 0; b < Num_bands; ++b) {



			for (int32_t i = 0, len = signal.area(); i < len; ++i) {
				signal_a->store(i, float2(signal.load(i), 0.f));
			}

		//	float alpha = 0.2f + 0.5f * (static_cast<float>(b) / static_cast<float>(Num_bands));

			float wl = static_cast<float>(spectral_data->wavelength_center(b));

			float f = 12.f;
			float alpha = 0.15f * (wl / 400.f) * (f / 18.f);

			fdft(*signal_b.get(), signal_a, alpha, 0, pool);
			fdft(*signal_a.get(), signal_b, alpha, 1, pool);
			squared_magnitude(signal_t.data(), signal_a->data(), resolution, resolution);

			float i_s = wl / 400.f;

			float normalization = (i_s * i_s);

			for (int32_t i = 0, len = signal.area(); i < len; ++i) {
				float s = 0.25f * normalization * signal_t.load(i);
				spectral_data[i].set_bin(b, s);
			}

			std::cout << b << "/" << Num_bands << " done" << std::endl;

		}

		pool.run_range([spectral_data, &float_image_a](int32_t begin, int32_t end) {
			for (int32_t i = begin; i < end; ++i) {
				auto& s = spectral_data[i];
				float3 linear_rgb = spectrum::XYZ_to_linear_RGB(s.normalized_XYZ());
				float_image_a.store(i, math::packed_float3(linear_rgb));
			}
		}, 0, resolution * resolution);

		delete [] spectral_data;
		*/
	} else {
		math::dft_2d(signal_f.data(), signal.data(), resolution, resolution, pool);

		centered_squared_magnitude(signal.data(), signal_f.data(), resolution, resolution);

		Spectrum* spectral_data = new Spectrum[resolution * resolution];

		pool.run_range([spectral_data, &signal, resolution](int32_t begin, int32_t end) {
			for (int32_t bin = begin; bin < end; ++bin) {
				diffraction(spectral_data, signal.data(), bin, resolution);
			}
		}, 0, Spectrum::num_bands());

		pool.run_range([spectral_data, &float_image_a](int32_t begin, int32_t end) {
			for (int32_t i = begin; i < end; ++i) {
				auto& s = spectral_data[i];
				float3 linear_rgb = spectrum::XYZ_to_linear_RGB(s.normalized_XYZ());
				float_image_a.store(i, math::packed_float3(linear_rgb));
			}
		}, 0, resolution * resolution);

		delete [] spectral_data;
	}

//	write_signal("signal_after.png", signal);

//	float radius = static_cast<float>(resolution) * 0.00390625f;
//	filter::Gaussian<math::packed_float3> gaussian(radius, radius * 0.0005f);
//	gaussian.apply(float_image_a);

	Byte_3 byte_image(Image::Description(Image::Type::Byte_3, dimensions));

	pool.run_range([&float_image_a, &byte_image](int32_t begin, int32_t end) {
		for (int32_t i = begin; i < end; ++i) {
			float3 linear_rgb = float3(float_image_a.load(i));
			byte3 srgb = spectrum::float_to_unorm(spectrum::linear_RGB_to_sRGB(linear_rgb));
			byte_image.store(i, srgb);
		}
	}, 0, resolution * resolution);

	encoding::png::Writer::write(near_field ? "near_field.png" : "far_field.png", byte_image);
}

void render_dirt(image::Float_1& signal) {
//	std::ifstream stream("lens_dirt.png", std::ios::binary);

//	try {
//		auto image = image::encoding::png::Reader::read(stream, image::Channels::X, 1);

//		if (image::Image::Type::Byte_1 != image->description().type) {
//			std::cout << "wrong type" << std::endl;
//		}

//		const image::Byte_1* byte_image = dynamic_cast<const image::Byte_1*>(image.get());

//		if (signal.description().dimensions != image->description().dimensions) {
//			std::cout << "dimensions not matching" << std::endl;
//			return;
//		}

//		for (int32_t i = 0, len = signal.area(); i < len; ++i) {
//			uint8_t byte = byte_image->load(i);
//			float linear = spectrum::unorm_to_float(byte);
//			signal.store(i, std::pow(linear, 4.f));
//		}

//	} catch (std::exception& e) {
//		std::cout << e.what() << std::endl;
//	}

//	return;

	Dirt dirt(signal.description().dimensions.xy, 4);

	dirt.set_brush(1.f);
	dirt.clear();

	rnd::Generator rng(0, 1, 2, 3);

	float dirt_radius = 0.003f;

	uint32_t num_base_dirt = 128;

	uint32_t base_scramble = rng.random_uint();

	for (uint32_t i = 0; i < num_base_dirt; ++i) {
		float2 p = math::hammersley(i, num_base_dirt, base_scramble);

		float rc = rng.random_float();
		dirt.set_brush(0.8f + 0.2f * rc);

		float rs = rng.random_float();
		dirt.draw_circle(p, dirt_radius + 32.f * rs * dirt_radius);
	}

	uint32_t num_detail_dirt = 2 * 1024;

	uint32_t detail_scramble = rng.random_uint();

	for (uint32_t i = 0; i < num_detail_dirt; ++i) {
		float2 p = math::hammersley(i, num_detail_dirt, detail_scramble);

		float rc = rng.random_float();
		dirt.set_brush(0.9f + 0.1f * rc);

		float rs = rng.random_float();
		dirt.draw_circle(p, dirt_radius + 2.f * rs * dirt_radius);
	}

	float inner = 1.f;
	float outer = 0.75f;

	dirt.draw_concentric_circles(float2(0.7f, 0.25f), 12, 0.005f, inner, outer);
	dirt.draw_concentric_circles(float2(0.4f, 0.6f), 12, 0.005f, inner, outer);
	dirt.draw_concentric_circles(float2(0.6f, 0.8f), 12, 0.005f, inner, outer);

	dirt.resolve(signal);

//	float radius = static_cast<float>(signal.description().dimensions.x) * 0.00390625f;
//	image::filter::Gaussian<float> gaussian(radius, radius * 0.0005f);
//	gaussian.apply(signal);
}

void render_aperture(const Aperture& aperture, image::Float_1& signal) {
	int32_t num_sqrt_samples = 4;
	std::vector<float2> kernel(num_sqrt_samples * num_sqrt_samples);

	float kd = 1.f / static_cast<float>(num_sqrt_samples);

	for (int32_t y = 0; y < num_sqrt_samples; ++y) {
		for (int32_t x = 0; x < num_sqrt_samples; ++x) {
			float2 k(0.5f * kd + static_cast<float>(x) * kd,
					 0.5f * kd + static_cast<float>(y) * kd);

			kernel[y * num_sqrt_samples + x] = k;
		}
	}

	int32_t resolution = signal.description().dimensions.x;
	float fr = static_cast<float>(resolution);
	float kn = 1.f / static_cast<float>(kernel.size());

	float radius = static_cast<float>(resolution - 2) / fr;

	for (int32_t y = 0; y < resolution; ++y) {
		for (int32_t x = 0; x < resolution; ++x) {
			float s = signal.load(x, y);

			float fx = static_cast<float>(x);
			float fy = static_cast<float>(y);

			float a = 0.f;
			for (auto k : kernel) {
				float2 p(-1.f + 2.f * ((fx + k.x) / fr),
						  1.f - 2.f * ((fy + k.y) / fr));

				a += kn * aperture.evaluate(p, radius);
			}

			signal.store(x, y, s * a);
		}
	}
}

void centered_squared_magnitude(float* result, const float2* source, size_t width, size_t height) {
	size_t row_size = math::dft_size(width);

	float fr = static_cast<float>(width);
	float normalization = 2.f / fr;

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

/*
		for (size_t x = 0, len = row_size - 1; x < len; ++x) {
			size_t o = y * row_size + x;
			float mag = math::squared_length(normalization * source[o]);

//			size_t a = ro * width + x + len;
//			std::cout << a << std::endl;

			result[ro * width + x + len] = mag;
		}

		for (size_t x = 0, len = row_size - 1; x < len; ++x) {
			size_t o = y * row_size + x;
			float mag = math::squared_length(normalization * source[o]);

//			size_t a = ro * width - x + len - 1;
//			std::cout << a << std::endl;

			result[ro * width - x + len - 1] = mag;
		}
*/

		for (size_t x = 0, len = row_size; x < len; ++x) {
			size_t o = y * row_size + x;

		//	float2 normalization = (0 == x || len - 1 == x) ? float2(1.f / fr, 2.f / fr) : float2(2.f / fr);

			float mag = math::squared_length(normalization * source[o]);

//			size_t a = ro * width + x + len - 1;
//			std::cout << o << ": " << a << std::endl;

			result[ro * width + x + len - 1] = mag;
		}

		for (size_t x = 1, len = row_size; x < len; ++x) {
			size_t o = y * row_size + x;

		//	float2 normalization(2.f / fr);

		//	float2 normalization = (len - 1 == x) ? float2(1.f / fr, 2.f / fr) : float2(2.f / fr);

			float mag = math::squared_length(normalization * source[o]);

//			size_t a = ro * width - x + len - 1;
//			std::cout << o << ": " << a << std::endl;

			result[ro * width - x + len - 1] = mag;
		}

//		std::cout << "row end" << std::endl;
	}
}

void squared_magnitude(float* result, const float2* source, size_t width, size_t height) {
	for (size_t i = 0, len = width * height; i < len; ++i) {
		float mag = math::squared_length(source[i]);
		result[i] = mag;
	}
}

void diffraction(Spectrum* result, const float* squared_magnitude,
				 uint32_t bin, int32_t resolution) {
	float fr = static_cast<float>(resolution);

	float wl_0 = Spectrum::wavelength_center(Spectrum::num_bands() - 1);
//	float wl_0 = Spectrum::wavelength_center(Spectrum::num_bands() / 2);

	float wl = Spectrum::wavelength_center(bin);

	float i_s = wl / wl_0;

	float normalization = (i_s * i_s);

	for (int32_t y = 0; y < resolution; ++y) {
		for (int32_t x = 0; x < resolution; ++x) {
			float2 p(-1.f + 2.f * ((static_cast<float>(x) + 0.5f) / fr),
					  1.f - 2.f * ((static_cast<float>(y) + 0.5f) / fr));

			float2 q = i_s * p;

			q.x =  0.5f * (q.x + 1.f);
			q.y = -0.5f * (q.y - 1.f);

		//	q = math::saturate(q);

			float r;

			if (q.x < 0.f || q.x >= 1.f
			||  q.y < 0.f || q.y >= 1.f) {
				r = 0.f;
			} else {
				float dp = math::dot(p, p);
				float v = std::max(1.f - dp * dp, 0.f);

				int32_t sx = static_cast<int32_t>(q.x * fr);
				int32_t sy = static_cast<int32_t>(q.y * fr);
				int32_t i = sy * resolution + sx;
				r = v * normalization * squared_magnitude[i];
			}

			int32_t o = y * resolution + x;
			result[o].set_bin(bin, r);
		}
	}
}

void write_signal(const std::string& name, const image::Float_1& signal) {
	auto d = signal.description().dimensions;

	image::Byte_1 image(image::Image::Description(image::Image::Type::Byte_1, d));

	for (int32_t i = 0, len = d.x * d.y; i < len; ++i) {
		float s = signal.load(i);
		uint8_t b = spectrum::float_to_unorm(s);
		image.store(i, b);
	}

	image::encoding::png::Writer::write(name, image);
}

float2 sqrtc(float2 c) {
	float l = math::length(c);
	return 0.7071067f * float2(std::sqrt(l + c.x), std::sqrt(l - c.x) * static_cast<float>(math::sign(c.y)));
}

float2 mulc(float2 a, float2 b) {
	return float2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

float2 mulc(float2 a, float t) {
	float c = std::cos(t);
	float s = std::sin(t);

	return float2(a.x * c - a.y * s, a.x * s + a.y * c);
}

void fdft(image::Float_2& destination, std::shared_ptr<image::Float_2> source,
		  float alpha, uint32_t mode, int32_t begin, int32_t end) {
	using namespace image::texture::sampler;
	Sampler_2D_linear<Address_mode_repeat> sampler;

	image::texture::Float_2 texture(source);

	auto d = source->description().dimensions;
	float2 df(d.xy);

	float m = static_cast<float>(d.x);
	float half_m = 0.5f * m;
	float sqrt_m = std::sqrt(m);
	float ss = 6.f;

	float cot = 1.f / std::tan(alpha * math::Pi * 0.5f);
	float csc = 1.f / std::sin(alpha * math::Pi * 0.5f);

	for (int32_t i = begin; i < end; ++i) {
		float2 coordinates = float2(source->coordinates_2(i)) + float2(0.5f, 0.5f);

		float2 uv = coordinates / df;

		float u = (coordinates.v[mode] - half_m) / sqrt_m;

		float2 integration(0.f);

		for (float k = -0.5f, dk = 1.f / (m * ss); k <= 0.5f; k += dk) {
			float2 kuv = uv;
			kuv.v[mode] = k + 0.5f;
			float2 g = sampler.sample_2(texture, kuv);

			float v = k * sqrt_m;
			float t = math::Pi * (cot * v * v - 2.f * csc * u * v);
			integration += mulc(g, t);
		}

		float2 s = mulc(sqrtc(float2(1.f, -cot)), math::Pi * cot * u * u);

		destination.store(i, mulc(s, integration) / (ss * sqrt_m));
	}
}

void fdft(image::Float_2& destination, std::shared_ptr<image::Float_2> source,
		  float alpha, uint32_t mode, thread::Pool& pool) {
	auto d = destination.description().dimensions;
	pool.run_range([&destination, source, alpha, mode](int32_t begin, int32_t end) {
		fdft(destination, source, alpha, mode, begin, end);
	}, 0, d.x * d.y);
}

}}
