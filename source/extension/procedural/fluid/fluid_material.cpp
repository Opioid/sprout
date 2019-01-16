#include "fluid_material.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "core/image/texture/texture.hpp"
#include "core/image/texture/texture_adapter.inl"
#include "core/image/texture/texture_byte_4_srgb.hpp"
#include "core/image/texture/texture_float_1.hpp"
#include "core/image/typed_image.hpp"
#include "core/scene/prop/prop.hpp"
#include "fluid_particle.hpp"
#include "fluid_vorton.hpp"
#include "volume_renderer.hpp"

#include "base/string/string.hpp"

#include <iostream>

namespace procedural::fluid {

using namespace image;

static int3 constexpr Visualization_dimensions(320);

Material::Material(Sampler_settings const& sampler_settings) noexcept
    : scene::material::volumetric::Grid_color(
          sampler_settings,
          Texture_adapter(std::make_shared<texture::Byte4_sRGB>(std::make_shared<Byte4>(
              Image::Description(Image::Type::Byte4, Visualization_dimensions))))),
      sim_(int3(256), Visualization_dimensions),
      current_frame_(0) {
    rnd::Generator rng(0, 0);

    for (uint32_t i = 0, len = sim_.num_vortons(); i < len; ++i) {
        Vorton& v = sim_.vortons()[i];

        //        float3 const r0(rng.random_float(), rng.random_float(), rng.random_float());

        //        v.position = 0.195f * (2.f * r0 - 1.f);

        //        float3 const r1(rng.random_float(), rng.random_float(), rng.random_float());

        //        v.vorticity = 64.f * normalize((2.f * r1 - 1.f));

        //        v.vorticity = 64.f * float3(0.f, 1.f, 0.f);

        float3 o;

        if (i < len / 2) {
            o = float3(-0.08f, 0.f, 0.f);
        } else {
            o = float3(0.08f, 0.f, 0.f);
        }

        float3 const dir = sample_sphere_uniform(float2(rng.random_float(), rng.random_float()));

        v.position = o + (0.05f + 0.01f * rng.random_float()) * dir;

        v.vorticity = 32.f * dir;
    }

    for (uint32_t i = 0, len = sim_.num_tracers(); i < len; ++i) {
        Particle& p = sim_.tracers()[i];

        float3 o;

        if (i < len / 2) {
            o       = float3(-0.08f, 0.f, 0.f);
            p.color = float3(0.3f, 0.8f, 0.3f);
        } else {
            o       = float3(0.08f, 0.f, 0.f);
            p.color = float3(0.8f, 0.3f, 0.3f);
        }

        float3 const dir = sample_sphere_uniform(float2(rng.random_float(), rng.random_float()));

        p.position = o + (0.05f + 0.01f * rng.random_float()) * dir;
    }
}

Material::~Material() noexcept {}

void Material::set_prop(scene::prop::Prop* prop) noexcept {
    prop_ = prop;
}

void Material::simulate(uint64_t      start, uint64_t /*end*/, uint64_t /*frame_length*/,
                        thread::Pool& pool) noexcept {
    // Hacky, but "moving" fluid props are not supported at the moment
    prop_->calculate_world_transformation();
    sim_.set_aabb(prop_->aabb());

    uint32_t const sim_frame = static_cast<uint32_t>(start / Simulation::Frame_length);

    std::cout << "Simulating..." << std::endl;

    for (uint32_t i = current_frame_, len = sim_frame; i < len; ++i) {
        auto const sim_start = std::chrono::high_resolution_clock::now();

        sim_.simulate(pool);

        std::cout << "Iteration " << i << " in "
                  << string::to_string(chrono::seconds_since(sim_start)) << " s" << std::endl;
    }

    current_frame_ = sim_frame;

    Volume_renderer renderer(color_.texture().dimensions_3(), 128);

    renderer.clear();

    for (uint32_t i = 0, len = sim_.num_tracers(); i < len; ++i) {
        auto const& tracer = sim_.tracers()[i];

        float3 const p = sim_.world_to_texture_point(tracer.position);

        renderer.splat(p, float4(tracer.color, 1.f));
    }

    renderer.resolve(*static_cast<image::Byte4*>(&color_.texture().image()));

    compile(pool);
}

bool Material::is_animated() const noexcept {
    return true;
}

Simulation& Material::simulation() noexcept {
    return sim_;
}

}  // namespace procedural::fluid
