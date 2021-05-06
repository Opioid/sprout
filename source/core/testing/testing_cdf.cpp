/*
#include "testing_cdf.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include "base/chrono/chrono.hpp"
#include "base/math/distribution_1d.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "base/string/string.hpp"
#include "file/file_system.hpp"
#include "image/encoding/rgbe/rgbe_reader.hpp"
#include "image/texture/texture_float_3.hpp"

namespace testing {
namespace cdf {

template <typename T, typename U>
void compare_distributions(T const& a, const U& b, float r);

template <typename T, typename U>
void assert_distributions(T const& a, const U& b, float2 r);

template <typename T>
void test_distribution(T const& d, const std::vector<float>& samples);

template <typename T>
void test_distribution(T const& d, const std::vector<float2>& samples);

template <typename T, typename U>
void assert_distributions(T const& a, const U& b, const std::vector<float2>& samples);

template <typename T>
void init(T& distribution, const image::texture::Float3& texture);

void test_1D() {
    std::cout << "testing::cdf::test_1D()" << std::endl;

    rnd::Generator rng(0, 0);

    //	size_t const num_values = 1024 * 10;

    //	std::vector<float> values(num_values);

    //	for (size_t i = 0, len = values.size(); i < len; ++i) {
    //		values[i] = 8.f * rng.random_float();
    //	}

    std::vector<float> values = {0.0f, 3.f, 0.0f, 0.f};

    //	size_t const num_samples = 1024 * 1024 * 32;

    //	std::vector<float> samples(num_samples);

    //	for (size_t i = 0, len = samples.size(); i < len; ++i) {
    //		samples[i] = rng.random_float();
    //	}

    math::Distribution_1D a;
    a.init(values.data(), uint32_t(values.size()));

    math::Distribution_lut_1D b;
    b.init(values.data(), uint32_t(values.size()));

    math::Distribution_implicit_pdf_lut_1D c;
    c.init(values.data(), uint32_t(values.size()));

    //	math::Distribution_implicit_pdf_lut_1D d;
    //	d.init(values.data(), uint32_t(values.size()), 8);

    math::Distribution_1D e;
    e.init(values.data(), uint32_t(values.size()));

    //	math::Distribution_1D f;
    //	f.init(values.data(), uint32_t(values.size()), 8);

    //	std::cout << "Distribution_1D" << std::endl;
    //	test_distribution(a, samples);

    //	std::cout << "Distribution_lut_1D" << std::endl;
    //	test_distribution(b, samples);

    //	std::cout << "Distribution_implicit_pdf_lut_1D(" << c.lut_size() << ")" << std::endl;
    //	test_distribution(c, samples);

    //	std::cout << "Distribution_implicit_pdf_lut_1D(" << d.lut_size() << ")" << std::endl;
    //	test_distribution(d, samples);

    //	std::cout << "Distribution_1D(" << e.lut_size() << ")" << std::endl;
    //	test_distribution(e, samples);

    //	std::cout << "Distribution_1D(" << f.lut_size() << ")" << std::endl;
    //	test_distribution(f, samples);

    compare_distributions(a, e, 0.3f);

    //	float pdf;
    //	float const result = e.sample_discrete(0.5f, pdf);
    //	std::cout << result << " " << pdf << std::endl;
}

void test_2D() {
    std::cout << "testing::cdf::test_2D()" << std::endl;

    image::encoding::rgbe::Reader reader;

    std::string const name = "../data/textures/uffizi_spherical.hdr";
    //	std::string const name = "../data/textures/river_road_spherical.hdr";
    //	std::string const name = "../data/textures/city_night_lights_spherical.hdr";
    //	std::string const name = "../data/textures/ennis_spherical.hdr";

    std::ifstream stream(name, std::ios::binary);
    if (!stream) {
        std::cout << "Could not read image" << std::endl;
        return;
    }

    rnd::Generator rng(0, 0);

    size_t const num_samples = 1024 * 1024 * 16;

    std::vector<float2> samples(num_samples);

    samples[0] = float2(0.f, 0.f);
    samples[1] = float2(0.f, 1.f);
    samples[2] = float2(1.f, 0.f);
    samples[3] = float2(1.f, 1.f);

    for (size_t i = 4, len = samples.size(); i < len; ++i) {
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

    math::Distribution_t_2D<math::Distribution_1D> d;
    init(d, texture);

    std::cout << "Distribution_2D" << std::endl;
    test_distribution(a, samples);

    std::cout << "Distribution_lut_2D" << std::endl;
    test_distribution(b, samples);

    std::cout << "Distribution_implicit_pdf_lut_2D" << std::endl;
    test_distribution(c, samples);

    std::cout << "Distribution_implicit_pdf_lut_lin_2D" << std::endl;
    test_distribution(d, samples);

    //	assert_distributions(a, d, samples);

    std::cout << "Done" << std::endl;
}

template <typename T, typename U>
void compare_distributions(T const& a, const U& b, float r) {
    auto const a_d = a.sample_discrete(r);

    auto const a_r = a.sample_continuous(r);

    float const a_pdf_2 = a.pdf(r);

    auto const b_d = b.sample_discrete(r);

    auto const b_r = b.sample_continuous(r);

    float const b_pdf_2 = b.pdf(r);

    std::cout << "a(" << r << "): " << a_d.offset << ", " << a_r.offset << ", " << a_d.pdf << ", "
              << a_r.pdf << ", " << a_pdf_2 << std::endl;

    std::cout << "b(" << r << "): " << b_d.offset << ", " << b_r.offset << ", " << b_d.pdf << ", "
              << b_r.pdf << ", " << b_pdf_2 << std::endl;
}

template <typename T, typename U>
void assert_distributions(T const& a, const U& b, float2 r) {
    float        a_pdf;
    float2 const a_r = a.sample_continuous(r, a_pdf);

    float        b_pdf;
    float2 const b_r = b.sample_continuous(r, b_pdf);

    if (a_r != b_r || std::abs(a_pdf - b_pdf) > 0.000001f) {
        std::cout << "a(" << r << "): " << a_r << ", " << a_pdf << std::endl;

        std::cout << "b(" << r << "): " << b_r << ", " << b_pdf << std::endl;
    }
}

template <typename T>
void test_distribution(T const& d, const std::vector<float>& samples) {
    auto const start = std::chrono::high_resolution_clock::now();

    float accumulated_r   = 0.f;
    float accumulated_pdf = 0.f;

    float const inv_num_samples = 1.f / float(samples.size());

    for (size_t i = 0, len = samples.size(); i < len; ++i) {
        float       pdf;
        float const r = d.sample_continuous(samples[i], pdf);

        accumulated_r += inv_num_samples * r;
        accumulated_pdf += inv_num_samples * pdf;
    }

    auto const duration = chrono::seconds_since(start);

    std::cout << "accumulated r: " << accumulated_r << std::endl;
    std::cout << "accumulated pdf: " << accumulated_pdf << std::endl;

    std::cout << "in " << string::to_string(duration) << " s" << std::endl;
}

template <typename T>
void test_distribution(T const& d, const std::vector<float2>& samples) {
    auto const start = std::chrono::high_resolution_clock::now();

    float2 accumulated_r   = 0.f;
    float  accumulated_pdf = 0.f;

    float const inv_num_samples = 1.f / float(samples.size());

    for (size_t i = 0, len = samples.size(); i < len; ++i) {
        auto const r = d.sample_continuous(samples[i]);

        accumulated_r += inv_num_samples * r.uv;
        accumulated_pdf += inv_num_samples * r.pdf;
    }

    auto const duration = chrono::seconds_since(start);

    std::cout << "accumulated r: " << accumulated_r << std::endl;
    std::cout << "accumulated pdf: " << accumulated_pdf << std::endl;

    std::cout << "in " << string::to_string(duration) << " s" << std::endl;
}

template <typename T, typename U>
void assert_distributions(T const& a, const U& b, const std::vector<float2>& samples) {
    for (size_t i = 0, len = samples.size(); i < len; ++i) {
        assert_distributions(a, b, samples[i]);
    }
}

template <typename T>
void init(T& distribution, const image::texture::Float3& texture) {
    auto const d = texture.dimensions_2();

    std::vector<typename T::Distribution_impl> conditional(d[1]);

    std::vector<float> luminance(d[0]);

    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            float3 const radiance = texture.at_3(x, y);

            luminance[x] = spectrum::luminance(radiance);
        }

        conditional[y].init(luminance.data(), d[0]);
    }

    distribution.init(conditional);
}

}  // namespace cdf
}  // namespace testing
*/
