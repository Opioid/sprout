#include "sampler_test.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "image/encoding/png/png_writer.hpp"
#include "image/image.hpp"
#include "image/procedural/image_renderer.hpp"
#include "sampler_golden_ratio.hpp"
#include "sampler_random.hpp"
#include "sampler_rd.hpp"

#include <iostream>
#include <string>
#include "base/math/print.hpp"

namespace sampler::testing {

using namespace sampler;
using namespace image;
using namespace image::procedural;

static float constexpr Dot_radius = 0.004f;

void render_set(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                Renderer& renderer, Byte3& target);

void render_disk(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                 Renderer& renderer, Byte3& target);

void render_triangle(std::string const& name, Sampler& sampler, uint32_t num_samples,
                     Renderer& renderer, Byte3& target);

void render_triangle_heitz(std::string const& name, Sampler& sampler, uint32_t num_samples,
                           RNG& rng, Renderer& renderer, Byte3& target);

void render_triangle_one(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                         Renderer& renderer, Byte3& target);

void render_quad(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                 Renderer& renderer, Byte3& target);

void render_quad(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                 float2 center, Renderer& renderer, Byte3& target);

// https://drive.google.com/file/d/1J-183vt4BrN9wmqItECIjjLIKwm29qSg/view
static inline float2 sample_triangle_uniform_heitz(float2 uv) {
    float2 t = 0.5f * uv;

    float const offset = t[1] - t[0];

    if (offset > 0.f) {
        t[1] += offset;
    } else {
        t[0] -= offset;
    }

    return t;
}

// https://pharr.org/matt/blog/2019/02/27/triangle-sampling-1.html
static inline float2 sample_triangle_uniform(float u) {
    uint32_t const ui = uint32_t(u * (1ull << 32));

    float2 a(1.f, 0.f);
    float2 b(0.f, 1.f);
    float2 c(0.f, 0.f);

    for (int32_t i = 0; i < 16; ++i) {
        int32_t d = (ui >> (2 * (15 - i))) & 0x3ull;

        float2 An;
        float2 Bn;
        float2 Cn;
        switch (d) {
            case 0:
                An = (b + c) / 2.f;
                Bn = (a + c) / 2.f;
                Cn = (a + b) / 2.f;
                break;
            case 1:
                An = a;
                Bn = (a + b) / 2.f;
                Cn = (a + c) / 2.f;
                break;
            case 2:
                An = (b + a) / 2.f;
                Bn = b;
                Cn = (b + c) / 2.f;
                break;
            case 3:
                An = (c + a) / 2.f;
                Bn = (c + b) / 2.f;
                Cn = c;
                break;
        }
        a = An;
        b = Bn;
        c = Cn;
    }

    return (a + b + c) / 3.f;
}

void test() {
    std::cout << "sampler::testing::test()" << std::endl;

    int2 constexpr dimensions(512, 512);

    Renderer renderer(dimensions, 4);

    Byte3 target((image::Description(dimensions)));

    uint32_t constexpr num_samples = 512;

    {
        rnd::Generator rng(0, 0);

        Golden_ratio sampler(1, 1);

        sampler.resize(num_samples);
        render_set("golden_ratio", sampler, num_samples, rng, renderer, target);
    }

    {
        rnd::Generator rng(0, 0);

        RD sampler(1, 1);

        sampler.resize(num_samples);
        render_set("rd", sampler, num_samples, rng, renderer, target);
    }

    {
        rnd::Generator rng(0, 0);

        Random sampler;

        sampler.resize(num_samples);
        render_set("random", sampler, num_samples, rng, renderer, target);
    }
}

void render_set(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                Renderer& renderer, Byte3& target) {
    //    render_disk(name + "_disk_0.png", sampler, renderer, target);
    //    render_disk(name + "_disk_1.png", sampler, renderer, target);

    //    render_triangle(name + "_triangle_0.png", sampler, renderer, target);
    //    render_triangle(name + "_triangle_1.png", sampler, renderer, target);

    //    render_triangle_heitz(name + "_triangle_heitz_0.png", sampler, renderer, target);
    //    render_triangle_heitz(name + "_triangle_heitz_1.png", sampler, renderer, target);

    //    render_triangle_one(name + "_triangle_one_0.png", sampler, renderer, target);
    //    render_triangle_one(name + "_triangle_one_1.png", sampler, renderer, target);

    render_quad(name + "_quad_0.png", sampler, num_samples, rng, renderer, target);
    render_quad(name + "_quad_1.png", sampler, num_samples, rng, renderer, target);
}

void render_disk(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                 Renderer& renderer, Byte3& target) {
    std::cout << name << ": ";

    float2 const center(0.5f, 0.5f);

    renderer.set_brush(float3(0.18f));
    renderer.clear();

    uint32_t const segment_len = num_samples / 4;

    float const n = 1.f / float(segment_len);

    sampler.start_pixel(rng);

    renderer.set_brush(float3(1.f, 0.f, 0.f));

    float2 average(0.f);
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ds = 0.5f * (sample_disk_concentric(s) + float2(1.f));
        average += n * ds;
        renderer.draw_circle(ds, Dot_radius);
    }

    std::cout << math::distance(average, center) << ", ";

    renderer.set_brush(float3(0.f, 0.7f, 0.f));

    average = float2(0.f);
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ds = 0.5f * (sample_disk_concentric(s) + float2(1.f));
        average += n * ds;
        renderer.draw_circle(ds, Dot_radius);
    }

    std::cout << math::distance(average, center) << ", ";

    renderer.set_brush(float3(0.f, 0.f, 1.f));

    average = float2(0.f);
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ds = 0.5f * (sample_disk_concentric(s) + float2(1.f));
        average += n * ds;
        renderer.draw_circle(ds, Dot_radius);
    }

    std::cout << math::distance(average, center) << ", ";

    renderer.set_brush(float3(0.7f, 0.7f, 0.f));

    average = float2(0.f);
    for (uint32_t i = 0, len = num_samples / 4; i < len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ds = 0.5f * (sample_disk_concentric(s) + float2(1.f));
        average += n * ds;
        renderer.draw_circle(ds, Dot_radius);
    }

    std::cout << math::distance(average, center) << std::endl;

    renderer.resolve_sRGB(target);

    encoding::png::Writer::write(name, target);
}

void render_triangle(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                     Renderer& renderer, Byte3& target) {
    std::cout << name << ": " << std::endl;

    renderer.set_brush(float3(0.18f));
    renderer.clear();

    uint32_t const segment_len = num_samples / 4;

    sampler.start_pixel(rng);

    renderer.set_brush(float3(1.f, 0.f, 0.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform(s);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.set_brush(float3(0.f, 0.7f, 0.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform(s);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.set_brush(float3(0.f, 0.f, 1.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform(s);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.set_brush(float3(0.7f, 0.7f, 0.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform(s);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.resolve_sRGB(target);

    encoding::png::Writer::write(name, target);
}

void render_triangle_heitz(std::string const& name, Sampler& sampler, uint32_t num_samples,
                           RNG& rng, Renderer& renderer, Byte3& target) {
    std::cout << name << ": " << std::endl;

    renderer.set_brush(float3(0.18f));
    renderer.clear();

    uint32_t const segment_len = num_samples / 4;

    sampler.start_pixel(rng);

    renderer.set_brush(float3(1.f, 0.f, 0.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform_heitz(s);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.set_brush(float3(0.f, 0.7f, 0.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform_heitz(s);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.set_brush(float3(0.f, 0.f, 1.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform_heitz(s);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.set_brush(float3(0.7f, 0.7f, 0.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform_heitz(s);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.resolve_sRGB(target);

    encoding::png::Writer::write(name, target);
}

void render_triangle_one(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                         Renderer& renderer, Byte3& target) {
    std::cout << name << ": " << std::endl;

    renderer.set_brush(float3(0.18f));
    renderer.clear();

    uint32_t const segment_len = num_samples / 4;

    sampler.start_pixel(rng);

    renderer.set_brush(float3(1.f, 0.f, 0.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform(s[1]);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.set_brush(float3(0.f, 0.7f, 0.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform(s[1]);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.set_brush(float3(0.f, 0.f, 1.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform(s[1]);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.set_brush(float3(0.7f, 0.7f, 0.f));

    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s  = sampler.sample_2D(rng);
        float2 const ts = sample_triangle_uniform(s[1]);

        renderer.draw_circle(ts, Dot_radius);
    }

    renderer.resolve_sRGB(target);

    encoding::png::Writer::write(name, target);
}

void render_quad(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                 Renderer& renderer, Byte3& target) {
    std::cout << name << std::endl;

    renderer.set_brush(float3(0.18f));
    renderer.clear();

    uint32_t const segment_len = num_samples / 4;

    sampler.start_pixel(rng);

    renderer.set_brush(float3(1.f, 0.f, 0.f));
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s = sampler.sample_2D(rng);
        //    std::cout << s << std::endl;
        renderer.draw_circle(s, Dot_radius);
    }

    renderer.set_brush(float3(0.f, 0.7f, 0.f));
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s = sampler.sample_2D(rng);
        //    std::cout << s << std::endl;
        renderer.draw_circle(s, Dot_radius);
    }

    renderer.set_brush(float3(0.f, 0.f, 1.f));
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s = sampler.sample_2D(rng);
        //    std::cout << s << std::endl;
        renderer.draw_circle(s, Dot_radius);
    }

    renderer.set_brush(float3(0.7f, 0.7f, 0.f));
    for (uint32_t i = 0; i < segment_len; ++i) {
        float2 const s = sampler.sample_2D(rng);
        //    std::cout << s << std::endl;
        renderer.draw_circle(s, Dot_radius);
    }

    renderer.resolve_sRGB(target);

    encoding::png::Writer::write(name, target);
}

void render_quad(std::string const& name, Sampler& sampler, uint32_t num_samples, RNG& rng,
                 float2 center, Renderer& renderer, Byte3& target) {
    renderer.set_brush(float3(0.18f));
    renderer.clear();

    sampler.start_pixel(rng);

    renderer.set_brush(float3(0.8f, 0.8f, 0.8f));
    for (uint32_t i = 0; i < num_samples; ++i) {
        float2 const s = sampler.sample_2D(rng);

        float2 const d = s - center;
        //		float const scale = 2.f * std::max(std::abs(d[0]), std::abs(d[1]));

        float const scale = length(d);

        float2 const p = scale * d + center;

        renderer.draw_circle(p, Dot_radius);
    }

    renderer.resolve_sRGB(target);

    encoding::png::Writer::write(name, target);
}

}  // namespace sampler::testing
