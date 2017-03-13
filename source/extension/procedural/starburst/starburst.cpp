#include "starburst.hpp"
#include "aperture.hpp"
#include "dirt.hpp"
#include "core/image/typed_image.inl"
#include "core/image/encoding/png/png_reader.hpp"
#include "core/image/encoding/png/png_writer.hpp"
#include "core/image/filter/image_gaussian.inl"
#include "core/image/procedural/image_renderer.inl"
#include "core/image/texture/texture_float_2.hpp"
#include "core/image/texture/sampler/sampler_linear_2d.inl"
#include "core/image/texture/sampler/bilinear.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/vector3.inl"
#include "base/math/fourier/dft.hpp"
#include "base/math/sampling/sample_distribution.hpp"
#include "base/random/generator.inl"
#include "base/spectrum/discrete.inl"
#include "base/spectrum/rgb.hpp"
#include "base/spectrum/xyz.inl"
#include "base/thread/thread_pool.hpp"

#include <iostream>
#include <fstream>
#include "base/math/print.hpp"

// Code for fractional DFT implementation
// http://onlinelibrary.wiley.com/store/10.1111/cgf.12953/asset/supinfo/cgf12953-sup-0002-S1.pdf?v=1&s=e3aaf53493a15c5111513bf1dbe1a6ee549ee804

namespace procedural { namespace starburst {

using namespace image;

static constexpr int32_t Num_bands = 64;

using Spectrum = spectrum::Discrete_spectral_power_distribution<Num_bands>;

void render_dirt(image::Float_1& signal);

void render_aperture(const Aperture& aperture, Float_1& signal);

template<typename Source, typename T>
static void fdft(Float_2& destination, const Source& source,
				 float alpha, thread::Pool& pool);

template<uint32_t Mode>
static void fdft(Float_2& destination, const Float_2& source,
				 float alpha, thread::Pool& pool);

void centered_squared_magnitude(float* result, const float2* source,
								int32_t width, int32_t height);

void squared_magnitude(float* result, const float2* source, int32_t width, int32_t height);

void squared_magnitude_transposed(float* result, const float2* source, int32_t dimension);

void diffraction(Spectrum* result, const float* source, int32_t bin, int32_t resolution);

void write_signal(const std::string& name, const Float_1& signal);

void create(thread::Pool& pool) {
	std::cout << "Starburst experiment" << std::endl;

	Spectrum::init(380.f, 720.f);

	int32_t resolution = 512;

	int2 dimensions(resolution, resolution);

	Float_1 signal(Image::Description(Image::Type::Float_1, dimensions));

	std::vector<float2> signal_f(resolution * math::dft_size(resolution));

	Float_3 float_image_a(Image::Description(Image::Type::Float_3, dimensions));

	bool dirt = false;
	if (dirt) {
		render_dirt(signal);
	} else {
		signal.clear(1.f);
	}

//	write_signal("signal.png", signal);

	Aperture aperture(8, 0.25f, 0.015f);
//	Aperture aperture(5, 0.f, 0.f);
	render_aperture(aperture, signal);

//	write_signal("signal.png", signal);

	bool near_field = true;

	if (near_field) {
		Image::Description description(Image::Type::Float_2, dimensions);

		Float_2 signal_a(description);
		Float_2 signal_b(description);

		float alpha = 0.12f;

		{
			fdft<Float_1, float>(signal_b, signal, alpha, pool);
			signal_b.square_transpose();
			fdft<Float_2, float2>(signal_a, signal_b, alpha, pool);
		//	squared_magnitude(signal.data(), signal_a.data(), resolution, resolution);
			squared_magnitude_transposed(signal.data(), signal_a.data(), resolution);
		}

		{
//			for (int32_t i = 0, len = signal.area(); i < len; ++i) {
//				signal_a.store(i, float2(signal.load(i), 0.f));
//			}

//			fdft<0>(signal_b, signal_a, alpha, pool);
//			fdft<1>(signal_a, signal_b, alpha, pool);
//			squared_magnitude(signal.data(), signal_a.data(), resolution, resolution);
		}

		pool.run_range([&float_image_a, &signal](uint32_t /*id*/, int32_t begin, int32_t end) {
			for (int32_t i = begin; i < end; ++i) {
				float s = 0.75f * signal.load(i);
				float_image_a.store(i, packed_float3(s));
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
				float_image_a.store(i, packed_float3(linear_rgb));
			}
		}, 0, resolution * resolution);

		delete[] spectral_data;
		*/
	} else {
		math::dft_2d(signal_f.data(), signal.data(), resolution, resolution, pool);

		centered_squared_magnitude(signal.data(), signal_f.data(), resolution, resolution);

		Spectrum* spectral_data = new Spectrum[resolution * resolution];

		pool.run_range([spectral_data, &signal, resolution]
			(uint32_t /*id*/, int32_t begin, int32_t end) {
				for (int32_t bin = begin; bin < end; ++bin) {
					diffraction(spectral_data, signal.data(), bin, resolution);
				}
			}, 0, Spectrum::num_bands());

		pool.run_range([spectral_data, &float_image_a]
			(uint32_t /*id*/, int32_t begin, int32_t end) {
				for (int32_t i = begin; i < end; ++i) {
					auto& s = spectral_data[i];
					float3 linear_rgb = spectrum::XYZ_to_linear_RGB(s.normalized_XYZ());
					float_image_a.store(i, packed_float3(linear_rgb));
				}
			}, 0, resolution * resolution);

		delete[] spectral_data;
	}

//	write_signal("signal_after.png", signal);

	float radius = static_cast<float>(resolution) * 0.00390625f;
	filter::Gaussian<packed_float3> gaussian(radius, radius * 0.0005f);
	gaussian.apply(float_image_a, pool);

	Byte_3 byte_image(Image::Description(Image::Type::Byte_3, dimensions));

	pool.run_range([&float_image_a, &byte_image](uint32_t /*id*/, int32_t begin, int32_t end) {
		for (int32_t i = begin; i < end; ++i) {
			float3 linear_rgb = float3(float_image_a.load(i));
			byte3 srgb = ::encoding::float_to_unorm(spectrum::linear_RGB_to_sRGB(linear_rgb));
			byte_image.store(i, srgb);
		}
	}, 0, resolution * resolution);

	image::encoding::png::Writer::write(near_field ? "near_field.png" : "far_field.png",
										byte_image);
}

void render_dirt(image::Float_1& signal) {
	Dirt dirt(signal.description().dimensions.xy(), 4);

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

//	float radius = static_cast<float>(signal.description().dimensions[0]) * 0.00390625f;
//	image::filter::Gaussian<float> gaussian(radius, radius * 0.0005f);
//	gaussian.apply(signal);
}

void render_aperture(const Aperture& aperture, Float_1& signal) {
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

	int32_t resolution = signal.description().dimensions[0];
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
				float2 p(-1.f + 2.f * ((fx + k[0]) / fr),
						  1.f - 2.f * ((fy + k[1]) / fr));

				a += kn * aperture.evaluate(p, radius);
			}

			signal.store(x, y, s * a);
		}
	}
}

void centered_squared_magnitude(float* result, const float2* source,
								int32_t width, int32_t height) {
	int32_t row_size = math::dft_size(width);

	float fr = static_cast<float>(width);
	float normalization = 2.f / fr;

	for (int32_t y = 0; y < height; ++y) {
		int32_t ro = y;

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

		for (int32_t x = 0, len = row_size; x < len; ++x) {
			int32_t o = y * row_size + x;

		//	float2 normalization = (0 == x || len - 1 == x) ? float2(1.f / fr, 2.f / fr) : float2(2.f / fr);

			float mag = math::squared_length(normalization * source[o]);

//			size_t a = ro * width + x + len - 1;
//			std::cout << o << ": " << a << std::endl;

			result[ro * width + x + len - 1] = mag;
		}

		for (int32_t x = 1, len = row_size; x < len; ++x) {
			int32_t o = y * row_size + x;

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

void squared_magnitude(float* result, const float2* source, int32_t width, int32_t height) {
	for (int32_t i = 0, len = width * height; i < len; ++i) {
		float mag = math::squared_length(source[i]);
		result[i] = mag;
	}
}

void squared_magnitude_transposed(float* result, const float2* source, int32_t dimension) {
	int32_t i = 0;
	for (int32_t y = 0; y < dimension; ++y) {
		for (int32_t x = 0; x < dimension; ++x, ++i) {
			float mag = math::squared_length(source[i]);
			int32_t o = x * dimension + y;
			result[o] = mag;
		}
	}
}

void diffraction(Spectrum* result, const float* squared_magnitude,
				 int32_t bin, int32_t resolution) {
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

			q[0] =  0.5f * (q[0] + 1.f);
			q[1] = -0.5f * (q[1] - 1.f);

		//	q = math::saturate(q);

			float r;

			if (q[0] < 0.f || q[0] >= 1.f
			||  q[1] < 0.f || q[1] >= 1.f) {
				r = 0.f;
			} else {
				float dp = math::dot(p, p);
				float v = std::max(1.f - dp * dp, 0.f);

				int32_t sx = static_cast<int32_t>(q[0] * fr);
				int32_t sy = static_cast<int32_t>(q[1] * fr);
				int32_t i = sy * resolution + sx;
				r = v * normalization * squared_magnitude[i];
			}

			int32_t o = y * resolution + x;
			result[o].set_bin(bin, r);
		}
	}
}

void write_signal(const std::string& name, const Float_1& signal) {
	const auto d = signal.description().dimensions;

	Byte_1 image(Image::Description(Image::Type::Byte_1, d));

	for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
		float s = signal.load(i);
		uint8_t b = ::encoding::float_to_unorm(s);
		image.store(i, b);
	}

	image::encoding::png::Writer::write(name, image);
}

static inline float sign(float x) {
	return x >= 0.f ? 1.f : -1.f;
}

static inline float2 sqrtc(float2 c) {
	float l = math::length(c);
	return 0.7071067f * float2(std::sqrt(l + c[0]), std::sqrt(l - c[0]) * sign(c[1]));
}

static inline float2 mulc(float2 a, float2 b) {
	return float2(a[0] * b[0] - a[1] * b[1], a[0] * b[1] + a[1] * b[0]);
}

static inline float2 mulc(float a, float t) {
	float c = std::cos(t);
	float s = std::sin(t);

	return float2(a * c, a * s);
}

static inline float2 mulc_cos_sin(float a, float2 cos_sin) {
	return float2(a * cos_sin[0], a * cos_sin[1]);
}

static inline float2 mulc_cos_sin(float2 a, float2 cos_sin) {
	return float2(a[0] * cos_sin[0] - a[1] * cos_sin[1], a[0] * cos_sin[1] + a[1] * cos_sin[0]);
}

static inline float2 mulc(float2 a, float t) {
	float c = std::cos(t);
	float s = std::sin(t);

	return float2(a[0] * c - a[1] * s, a[0] * s + a[1] * c);
}

static float map(int32_t b, float tc, int2& x_x1) {
	const float u = tc - 0.5f;

	const float fu = std::floor(u);

	const int32_t x = static_cast<int32_t>(fu);

	x_x1[0] = x < 0 ? 0 : x;
	x_x1[1] = x >= b ? b : x + 1;

	return u - fu;
}

template<typename T>
static inline T linear(T c[2], float s) {
	const float _s = 1.f - s;

	return _s * c[0] + s * c[1];
}

template<typename Source, typename T>
static T sample(const Source& source, int32_t b, float tc, int32_t y) {
	int2 x_x1;
	float s = map(b, tc, x_x1);

	T c[2];
	source.pair<0>(x_x1, y, c);
	return linear(c, s);
}

template<uint32_t Axis>
static float2 sample_2(const Float_2& source, int32_t b, float tc, int32_t y) {
	int2 x_x1;
	float s = map(b, tc, x_x1);

	float2 c[2];
	source.pair<Axis>(x_x1, y, c);
	return linear(c, s);
}

template<typename Source, typename T>
static void fdft(Float_2& destination, const Source& source,
				 float alpha, int32_t begin, int32_t end) {
	const int32_t d = destination.description().dimensions[0];
	const int32_t b = d - 1;

	const float m = float(d);
	const float half_m = 0.5f * m;
	const float sqrt_m = std::sqrt(m);
	const float i_sqrt_m = 1.f / sqrt_m;
	const float iss = std::floor(1.f / alpha);// 8.f;
	const float norm = 1.f / (iss * sqrt_m);
	const float dk = 1.f / (m * iss);

	const float ucot = 1.f / std::tan(alpha * (math::Pi * 0.5f));
	const float cot = math::Pi * ucot;
	const float csc = (2.f * math::Pi) / std::sin(alpha * (math::Pi * 0.5f));

	const float2 sx = sqrtc(float2(1.f, -ucot));

	class Row {

	public:

		Row(uint32_t width, uint32_t num_samples) :
			width_(width),
			num_samples_(num_samples),
			cos_sin_(new float2[width * num_samples]),
			s_(new float2[width]) {}

		~Row() {
			delete[] s_;
			delete[] cos_sin_;
		}

		void init(float half_m, float dk, float csc,
				  float cot, float sqrt_m, float i_sqrt_m,
				  float2 sx) {
			float coordinates = 0.5f;
			for (uint32_t x = 0, len = width_; x < len; ++x, ++coordinates)	{
				const float u = (coordinates - half_m) * i_sqrt_m;
				const float cscu = csc * u;

				uint32_t ss = 0;
				for (float k = -0.5f; k <= 0.5f; k += dk, ++ss) {
					const float v = k * sqrt_m;
					const float t = v * (cot * v - cscu);
					const uint32_t i = x * num_samples_ + ss;
					cos_sin_[i][0] = std::cos(t);
					cos_sin_[i][1] = std::sin(t);

//					if (ss >= num_samples_) {
//				//		std::cout << "alarmio" << std::endl;

//						std::cout << ss << " of " << num_samples_ << std::endl;
//					}
				}

		//		if (0 == x) std::cout << ss << std::endl;

				s_[x] = mulc(sx, cot * u * u);
			}
		}

		float2 cos_sin(uint32_t x, uint32_t ss) const {
			uint32_t i = x * num_samples_ + ss;
			return cos_sin_[i];
		}

		float2 s(uint32_t x) const {
			return s_[x];
		}

	private:

		uint32_t width_;
		uint32_t num_samples_;
		float2* cos_sin_;
		float2* s_;
	};

	const uint32_t num_samples = static_cast<uint32_t>(m * iss) + 1;// + 0.5f);

//	std::cout << "provide: " << num_samples << std::endl;

	Row row(d, num_samples);
	row.init(half_m, dk, csc, cot, sqrt_m, i_sqrt_m, sx);

	T* row_samples = new T[num_samples];

	for (int32_t y = begin; y < end; ++y) {
		uint32_t ss = 0;
		for (float k = -0.5f; k <= 0.5f; k += dk, ++ss) {
			const float tc = (k + 0.5f) * m;
			row_samples[ss] = sample<Source, T>(source, b, tc, y);
		}

		float coordinates = 0.5f;
		for (int32_t x = 0; x < d; ++x, ++coordinates) {
			float2 integration(0.f);

			for (uint32_t i = 0, len = num_samples; i < len; ++i) {
				const T g = row_samples[i];

				integration += mulc_cos_sin(g, row.cos_sin(x, i));
			}

	//		const float  u = (coordinates - half_m) * i_sqrt_m;
	//		const float2 s = mulc(sx, cot * u * u);

			const float2 s = row.s(x);

			destination.store(x, y, norm * mulc(s, integration));
		}
	}

	delete[] row_samples;
}

template<typename Source, typename T>
static void fdft(Float_2& destination, const Source& source,
				 float alpha, thread::Pool& pool) {
	const auto d = destination.description().dimensions;
	pool.run_range([&destination, &source, alpha]
		(uint32_t /*id*/, int32_t begin, int32_t end) {
			fdft<Source, T>(destination, source, alpha, begin, end);
		}, 0, d[1]);
}


template<uint32_t Mode>
static void fdft(Float_2& destination, const Float_2& source,
				 float alpha, int32_t begin, int32_t end) {
	const int2 d = destination.description().dimensions.xy();
	const int2 b = d - int2(1);

	const float m = float(d[Mode]);
	const float half_m = 0.5f * m;
	const float sqrt_m = std::sqrt(m);
	const float i_sqrt_m = 1.f / sqrt_m;
	const float ss = std::floor(1.f / alpha);// 8.f;
	const float norm = 1.f / (ss * sqrt_m);
	const float dk = 1.f / (m * ss);

	const float ucot = 1.f / std::tan(alpha * (math::Pi * 0.5f));
	const float cot = math::Pi * ucot;
	const float csc = (2.f * math::Pi) / std::sin(alpha * (math::Pi * 0.5f));

	const float2 sx = sqrtc(float2(1.f, -ucot));

	float2 coordinates = float2(0.5f, static_cast<float>(begin) + 0.5f);

	int32_t  filter_y;

	for (int32_t y = begin; y < end; ++y, ++coordinates[1]) {
		coordinates[0] = 0.5f;

		if (0 == Mode) {
			filter_y = y;
		}

		for (int32_t x = 0; x < d[0]; ++x, ++coordinates[0]) {
			if (1 == Mode) {
				filter_y = x;
			}

			const float u = (coordinates[Mode] - half_m) * i_sqrt_m;
			const float cscu = csc * u;

			float2 integration(0.f);

			for (float k = -0.5f; k <= 0.5f; k += dk) {
				const float tc = (k + 0.5f) * m;
				const float2 g = sample_2<Mode>(source, b[Mode], tc, filter_y);

				const float v = k * sqrt_m;
				const float t = v * (cot * v - cscu);

				integration += mulc(g, t);
			}

			const float2 s = mulc(sx, cot * u * u);

			destination.store(x, y, norm * mulc(s, integration));
		}
	}
}

template<uint32_t Mode>
static void fdft(Float_2& destination, const Float_2& source,
				 float alpha, thread::Pool& pool) {
	const auto d = destination.description().dimensions;
	pool.run_range([&destination, &source, alpha]
		(uint32_t /*id*/, int32_t begin, int32_t end) {
			fdft<Mode>(destination, source, alpha, begin, end);
		}, 0, d[1]);
}

}}
