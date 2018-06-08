#include "sampler_test.hpp"
#include "base/math/sampling/sampling.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "image/procedural/image_renderer.hpp"
#include "image/typed_image.inl"
#include "sampler_ems.hpp"
#include "sampler_golden_ratio.hpp"
#include "sampler_hammersley.hpp"
#include "sampler_random.hpp"

#include <iostream>
#include <string>
#include "base/math/print.hpp"

namespace sampler {
namespace testing {

using namespace sampler;
using namespace image;
using namespace image::procedural;

void render_set(std::string const& name, Sampler& sampler, Renderer& renderer, Byte3& target);

void render_disk(std::string const& name, Sampler& sampler, rnd::Generator& scramble,
                 Renderer& renderer, Byte3& target);

void render_quad(std::string const& name, Sampler& sampler, rnd::Generator& scramble,
                 Renderer& renderer, Byte3& target);

void render_quad(std::string const& name, Sampler& sampler, rnd::Generator& scramble, float2 center,
                 Renderer& renderer, Byte3& target);

void test() {
    std::cout << "sampler::testing::test()" << std::endl;

    const int2 dimensions(512, 512);
    Renderer   renderer(dimensions, 4);
    Byte3      target(Image::Description(Image::Type::Byte3, dimensions));

    uint32_t num_samples = 256;

    //	{
    //		rnd::Generator rng;
    //		EMS sampler(rng);
    //		sampler.resize(num_samples, 1, 1, 1);
    //		render_set("ems", sampler, renderer, target);
    //	}
    //	{
    //		rnd::Generator rng;
    //		Golden_ratio sampler(rng);
    //		sampler.resize(num_samples, 1, 1, 1);
    //		render_set("golden_ratio", sampler, renderer, target);
    //	}
    //	{
    //		rnd::Generator rng;
    //		Random sampler(rng);
    //		sampler.resize(num_samples, 1, 1, 1);
    //		render_set("random_disk", sampler, renderer, target);
    //	}
    //	{
    //		rnd::Generator rng;
    //		Hammersley sampler(rng);
    //		sampler.resize(num_samples, 1, 1, 1);
    //		render_set("hammersley", sampler, renderer, target);
    //	}
    {
        rnd::Generator rng;
        Hammersley     sampler(rng);
        sampler.resize(num_samples, 1, 1, 1);
        float2 const center(0.2f, 0.5f);
        render_quad("hammersley_quad_0.png", sampler, rng, center, renderer, target);
    }
}

void render_set(std::string const& name, Sampler& sampler, Renderer& renderer, Byte3& target) {
    rnd::Generator rng(0, 1);

    render_disk(name + "_disk_0.png", sampler, rng, renderer, target);
    render_disk(name + "_disk_1.png", sampler, rng, renderer, target);

    render_quad(name + "_quad_0.png", sampler, rng, renderer, target);
    render_quad(name + "_quad_1.png", sampler, rng, renderer, target);
}

void render_disk(std::string const& name, Sampler& sampler, rnd::Generator& scramble,
                 Renderer& renderer, Byte3& target) {
    std::cout << name << ": ";

    float2 const center(0.5f, 0.5f);

    renderer.set_brush(float3(0.18f));
    renderer.clear();

    uint32_t const num_samples = sampler.num_samples();

    uint32_t const segment_len = num_samples / 4;

    float const n = 1.f / static_cast<float>(segment_len);

    sampler.resume_pixel(0, scramble);

    renderer.set_brush(float3(1.f, 0.f, 0.f));

    float2 average(0.f);
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.generate_sample_2D();
        float2 const ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
        average += n * ds;
        renderer.draw_circle(ds, 0.005f);
    }

    std::cout << math::distance(average, center) << ", ";

    renderer.set_brush(float3(0.f, 0.7f, 0.f));

    average = float2(0.f);
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.generate_sample_2D();
        float2 const ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
        average += n * ds;
        renderer.draw_circle(ds, 0.005f);
    }

    std::cout << math::distance(average, center) << ", ";

    renderer.set_brush(float3(0.f, 0.f, 1.f));

    average = float2(0.f);
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.generate_sample_2D();
        float2 const ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
        average += n * ds;
        renderer.draw_circle(ds, 0.005f);
    }

    std::cout << math::distance(average, center) << ", ";

    renderer.set_brush(float3(0.7f, 0.7f, 0.f));

    average = float2(0.f);
    for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
        float2 const s  = sampler.generate_sample_2D();
        float2 const ds = 0.5f * (math::sample_disk_concentric(s) + float2(1.f));
        average += n * ds;
        renderer.draw_circle(ds, 0.005f);
    }

    std::cout << math::distance(average, center) << std::endl;

    renderer.resolve_sRGB(target);

    encoding::png::Writer::write(name, target);
}

void render_quad(std::string const& name, Sampler& sampler, rnd::Generator& scramble,
                 Renderer& renderer, Byte3& target) {
    renderer.set_brush(float3(0.18f));
    renderer.clear();

    uint32_t const num_samples = sampler.num_samples();

    uint32_t const segment_len = num_samples / 4;

    sampler.resume_pixel(0, scramble);

    renderer.set_brush(float3(1.f, 0.f, 0.f));
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s = sampler.generate_sample_2D();
        renderer.draw_circle(s, 0.005f);
    }

    renderer.set_brush(float3(0.f, 0.7f, 0.f));
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s = sampler.generate_sample_2D();
        renderer.draw_circle(s, 0.005f);
    }

    renderer.set_brush(float3(0.f, 0.f, 1.f));
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s = sampler.generate_sample_2D();
        renderer.draw_circle(s, 0.005f);
    }

    renderer.set_brush(float3(0.7f, 0.7f, 0.f));
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s = sampler.generate_sample_2D();
        renderer.draw_circle(s, 0.005f);
    }

    renderer.resolve_sRGB(target);

    encoding::png::Writer::write(name, target);
}

void render_quad(std::string const& name, Sampler& sampler, rnd::Generator& scramble, float2 center,
                 Renderer& renderer, Byte3& target) {
    renderer.set_brush(float3(0.18f));
    renderer.clear();

    uint32_t const num_samples = sampler.num_samples();

    sampler.resume_pixel(0, scramble);

    renderer.set_brush(float3(0.8f, 0.8f, 0.8f));
    for (uint32_t i = 0; i < num_samples; ++i) {
        float2 const s = sampler.generate_sample_2D();

        float2 const d = s - center;
        //		float const scale = 2.f * std::max(std::abs(d[0]), std::abs(d[1]));

        float const scale = math::length(d);

        float2 const p = scale * d + center;

        renderer.draw_circle(p, 0.005f);
    }

    renderer.resolve_sRGB(target);

    encoding::png::Writer::write(name, target);
}

}  // namespace testing
}  // namespace sampler
