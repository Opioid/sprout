#include "testing_cdf.hpp"
#include "file/file_system.hpp"
#include "image/encoding/rgbe/rgbe_reader.hpp"
#include "image/texture/texture_float_3.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/distribution/distribution_2d.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "base/string/string.hpp"
#include <vector>
#include <fstream>
#include <iostream>

namespace testing { namespace cdf {

template<typename T>
void test_distribution(const T& d, const std::vector<float>& samples);

template<typename T>
void test_distribution(const T& d, const std::vector<float2>& samples);

template<typename T>
void init(T& distribution, const image::texture::Float3& texture);

void test_1D() {
	std::cout << "testing::cdf::test_1d()" << std::endl;

	rnd::Generator rng;

	const size_t num_values = 1024 * 10;

	std::vector<float> values(num_values);

	for (size_t i = 0, len = values.size(); i < len; ++i) {
		values[i] = 8.f * rng.random_float();
	}

	const size_t num_samples = 1024 * 1024 * 32;

	std::vector<float> samples(num_samples);

	for (size_t i = 0, len = samples.size(); i < len; ++i) {
		samples[i] = rng.random_float();
	}



	math::Distribution_1D a;
	a.init(values.data(), static_cast<uint32_t>(num_values));

	math::Distribution_lut_1D b;
	b.init(values.data(), static_cast<uint32_t>(num_values));

	math::Distribution_implicit_pdf_lut_1D c;
	c.init(values.data(), static_cast<uint32_t>(num_values));

	math::Distribution_implicit_pdf_lut_1D d;
	d.init(values.data(), static_cast<uint32_t>(num_values), 8);

	math::Distribution_implicit_pdf_lut_lin_1D e;
	e.init(values.data(), static_cast<uint32_t>(num_values));


	std::cout << "Distribution_1D" << std::endl;
	test_distribution(a, samples);

	std::cout << "Distribution_lut_1D" << std::endl;
	test_distribution(b, samples);

	std::cout << "Distribution_implicit_pdf_lut_1D(" << c.lut_size() << ")" << std::endl;
	test_distribution(c, samples);

	std::cout << "Distribution_implicit_pdf_lut_1D(" << d.lut_size() << ")" << std::endl;
	test_distribution(d, samples);

	std::cout << "Distribution_implicit_pdf_lut_lin_1D(" << e.lut_size() << ")" << std::endl;
	test_distribution(e, samples);

//	std::cout << "Distribution_lut_1D" << std::endl;
//	test_distribution(b, samples);

//	std::cout << "Distribution_1D" << std::endl;
//	test_distribution(a, samples);

//	std::cout << "Distribution_implicit_pdf_lut_1D" << std::endl;
//	test_distribution(c, samples);

}

void test_2D() {
	image::encoding::rgbe::Reader reader;

	const std::string name = "../data/textures/river_road_spherical.hdr";
//	const std::string name = "../data/textures/ennis_spherical.hdr";

	std::ifstream stream(name, std::ios::binary);
	if (!stream) {
		std::cout << "Could not read image" << std::endl;
		return;
	}

	rnd::Generator rng;

	const size_t num_samples = 1024 * 1024 * 16;

	std::vector<float2> samples(num_samples);

	for (size_t i = 0, len = samples.size(); i < len; ++i) {
		samples[i] = float2(rng.random_float(), rng.random_float());
	}

	auto image = reader.read(stream);

	const image::texture::Float3 texture(image);

	math::Distribution_t_2D<math::Distribution_1D> a;
	init(a, texture);

	math::Distribution_t_2D<math::Distribution_lut_1D> b;
	init(b, texture);

	math::Distribution_t_2D<math::Distribution_implicit_pdf_lut_1D> c;
	init(c, texture);

	math::Distribution_t_2D<math::Distribution_implicit_pdf_lut_lin_1D> d;
	init(d, texture);

	std::cout << "Distribution_2D" << std::endl;
	test_distribution(a, samples);

	std::cout << "Distribution_lut_2D" << std::endl;
	test_distribution(b, samples);

	std::cout << "Distribution_implicit_pdf_lut_2D" << std::endl;
	test_distribution(c, samples);

	std::cout << "Distribution_implicit_pdf_lut_lin_2D" << std::endl;
	test_distribution(d, samples);
}

template<typename T>
void test_distribution(const T& d, const std::vector<float>& samples) {
	const auto start = std::chrono::high_resolution_clock::now();

	float accumulated_r = 0.f;
	float accumulated_pdf = 0.f;

	const float inv_num_samples = 1.f / static_cast<float>(samples.size());

	for (size_t i = 0, len = samples.size(); i < len; ++i) {
		float pdf;
		const float r = d.sample_continuous(samples[i], pdf);

		accumulated_r += inv_num_samples * r;
		accumulated_pdf += inv_num_samples * pdf;
	}

	const auto duration = chrono::seconds_since(start);

	std::cout << "accumulated r: " << accumulated_r << std::endl;
	std::cout << "accumulated pdf: " << accumulated_pdf << std::endl;

	std::cout << "in " << string::to_string(duration) << " s" << std::endl;
}

template<typename T>
void test_distribution(const T& d, const std::vector<float2>& samples) {
	const auto start = std::chrono::high_resolution_clock::now();

	float2 accumulated_r = 0.f;
	float accumulated_pdf = 0.f;

	const float inv_num_samples = 1.f / static_cast<float>(samples.size());

	for (size_t i = 0, len = samples.size(); i < len; ++i) {
		float pdf;
		const float2 r = d.sample_continuous(samples[i], pdf);

		accumulated_r += inv_num_samples * r;
		accumulated_pdf += inv_num_samples * pdf;
	}

	const auto duration = chrono::seconds_since(start);

	std::cout << "accumulated r: " << accumulated_r << std::endl;
	std::cout << "accumulated pdf: " << accumulated_pdf << std::endl;

	std::cout << "in " << string::to_string(duration) << " s" << std::endl;
}

template<typename T>
void init(T& distribution, const image::texture::Float3& texture) {
	const auto d = texture.dimensions_2();

	std::vector<typename T::Distribution_impl> conditional(d[1]);

	std::vector<float> luminance(d[0]);

	for (int32_t y = 0; y < d[1]; ++y) {
		for (int32_t x = 0; x < d[0]; ++x) {
			const float3 radiance = texture.at_3(x, y);

			luminance[x] = spectrum::luminance(radiance);
		}

		conditional[y].init(luminance.data(), d[0]);
	}

	distribution.init(conditional);
}

}}
