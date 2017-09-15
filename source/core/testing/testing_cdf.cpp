#include "testing_cdf.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/random/generator.inl"
#include "base/string/string.hpp"
#include <vector>
#include <iostream>

namespace testing { namespace cdf {

template<typename T>
void test_distribution(const T& d, const std::vector<float>& samples);

void test_1d() {
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
	d.init(values.data(), static_cast<uint32_t>(num_values), num_values / 8);


	std::cout << "Distribution_1D" << std::endl;
	test_distribution(a, samples);

	std::cout << "Distribution_lut_1D" << std::endl;
	test_distribution(b, samples);

	std::cout << "Distribution_implicit_pdf_lut_1D(" << c.lut_size() << ")" << std::endl;
	test_distribution(c, samples);

	std::cout << "Distribution_implicit_pdf_lut_1D(" << d.lut_size() << ")" << std::endl;
	test_distribution(d, samples);

//	std::cout << "Distribution_lut_1D" << std::endl;
//	test_distribution(b, samples);

//	std::cout << "Distribution_1D" << std::endl;
//	test_distribution(a, samples);

//	std::cout << "Distribution_implicit_pdf_lut_1D" << std::endl;
//	test_distribution(c, samples);

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

}}
