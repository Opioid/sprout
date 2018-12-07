#include "flakes_provider.hpp"
#include "base/math/sample_distribution.inl"
#include "base/math/sampling.inl"
#include "base/math/vector4.inl"
#include "base/memory/variant_map.inl"
#include "base/random/generator.inl"
#include "image/encoding/png/png_writer.hpp"
#include "image/procedural/image_renderer.hpp"
#include "image/typed_image.hpp"

namespace image::procedural::flakes {

std::shared_ptr<Image> Provider::create_normal_map(memory::Variant_map const& options) noexcept {
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

    std::shared_ptr<Byte3> image = std::make_shared<Byte3>(
        Image::Description(Image::Type::Byte3, props.dimensions));

    renderer.resolve(*image);

    encoding::png::Writer::write("flakes_normal.png", *image);

    return image;
}

std::shared_ptr<Image> Provider::create_mask(memory::Variant_map const& options) noexcept {
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

    std::shared_ptr<Byte1> image = std::make_shared<Byte1>(
        Image::Description(Image::Type::Byte1, props.dimensions));

    renderer.resolve(*image);

    encoding::png::Writer::write("flakes_mask.png", *image);

    return image;
}

Provider::Properties::Properties(memory::Variant_map const& options) noexcept
    : dimensions(1024, 1024) {
    float size    = 0.006f;
    float density = 0.5f;
    options.query("size", size);
    options.query("density", density);

    radius = 0.5f * size;

    num_flakes = static_cast<uint32_t>(density / (size * size) + 0.5f);
}

Provider::Flake Provider::random_flake(uint32_t index, uint32_t seed, Properties const& props,
                                       rnd::Generator& rng) noexcept {
    float2 const s0 = math::thing(index, props.num_flakes, seed);

    float2 const s1 = float2(rng.random_float(), rng.random_float());

    float3 const normal = normalize(math::sample_hemisphere_uniform(s1) + float3(0.f, 0.f, 1.75f));

    return Flake{s0, normal};
}

}  // namespace image::procedural::flakes
