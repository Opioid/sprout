#include "flakes_provider.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "base/memory/variant_map.inl"
#include "base/random/generator.inl"
#include "image/encoding/png/png_writer.hpp"
#include "image/image.hpp"
#include "image/procedural/image_renderer.hpp"

namespace image::procedural::flakes {

Image* Provider::create_normal_map(Variants const& options) {
    Properties props(options);

    Renderer renderer(props.dimensions, 1);

    renderer.set_brush(float3(0.f, 0.f, 1.f));
    renderer.clear();

    rnd::Generator rng(0, 0);

    uint32_t const seed = rng.random_uint();

    for (uint32_t i = 0; i < props.num_flakes; ++i) {
        Flake const flake = random_flake(i, seed, props, rng);

        renderer.set_brush(flake.normal);

        //   renderer.draw_circle(flake.pos, props.radius, 1);

        renderer.draw_disk(flake.pos, flake.normal, props.radius, 1);

        //   renderer.draw_bounding_square(flake.pos, props.radius);
    }

    Image* image = new Image(Byte3(Description(props.dimensions)));

    renderer.resolve(image->byte3());

    encoding::png::Writer::write("flakes_normal.png", image->byte3());

    return image;
}

Image* Provider::create_mask(Variants const& options) {
    Properties props(options);

    Renderer renderer(props.dimensions, 8);

    renderer.set_brush(float3(0.f));
    renderer.clear();

    rnd::Generator rng(0, 0);

    uint32_t const seed = rng.random_uint();

    renderer.set_brush(float3(1.f));

    for (uint32_t i = 0; i < props.num_flakes; ++i) {
        Flake const flake = random_flake(i, seed, props, rng);

        renderer.draw_disk(flake.pos, flake.normal, props.radius);
    }

    Image* image = new Image(Byte1(Description(props.dimensions)));

    renderer.resolve(image->byte1());

    encoding::png::Writer::write("flakes_mask.png", image->byte1());

    return image;
}

Provider::Properties::Properties(Variants const& options) : dimensions(1024, 1024) {
    float const size = options.query("size", 0.006f);
    float const density = options.query("density", 0.5f);

    radius = 0.5f * size;

    num_flakes = uint32_t(std::lrint(density / (size * size)));
}

Provider::Flake Provider::random_flake(uint32_t index, uint32_t seed, Properties const& props,
                                       RNG& rng) {
    float2 const s0 = math::thing(index, props.num_flakes, seed);

    float2 const s1 = float2(rng.random_float(), rng.random_float());

    float3 const normal = normalize(sample_hemisphere_uniform(s1) + float3(0.f, 0.f, 1.75f));

    return Flake{s0, normal};
}

}  // namespace image::procedural::flakes
