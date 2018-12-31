#include "fluid_material.hpp"
#include "base/chrono/chrono.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "core/image/texture/texture.hpp"
#include "core/image/texture/texture_adapter.inl"
#include "core/image/typed_image.hpp"
#include "fluid_particle.hpp"
#include "fluid_vorton.hpp"
#include "volume_renderer.hpp"

#include "base/string/string.hpp"

#include <iostream>

namespace procedural::fluid {

Material::Material(Sampler_settings const& sampler_settings,
                   Texture_adapter const&  density) noexcept
    : scene::material::volumetric::Grid(sampler_settings, density),
	  sim_(int3(128)),
      current_frame_(0) {
    rnd::Generator rng(0, 0);

    for (uint32_t i = 0, len = sim_.num_vortons(); i < len; ++i) {
        Vorton& v = sim_.vortons()[i];

        v.radius = 0.01f;

        float3 const p(rng.random_float(), rng.random_float(), rng.random_float());

        v.position = 0.2f * (2.f * p - 1.f);

        float3 const vorticity = 2.f * float3(rng.random_float(), rng.random_float(),
                                              rng.random_float()) -
                                 1.f;

        v.vorticity = 16.f * normalize(vorticity);
    }

    for (uint32_t i = 0, len = sim_.num_tracers(); i < len; ++i) {
        float3 const p(rng.random_float(), rng.random_float(), rng.random_float());

        sim_.tracers()[i].position = (0.05f + 0.01f * rng.random_float()) *
                                     normalize(2.f * p - 1.f);
    }
}

Material::~Material() noexcept {}

void Material::simulate(uint64_t start, uint64_t /*end*/, uint64_t frame_length,
                        thread::Pool& pool) noexcept {
    uint32_t const sim_frame = static_cast<uint32_t>(start / frame_length);

    std::cout << "Simulating..." << std::endl;

    for (uint32_t i = current_frame_, len = sim_frame; i < len; ++i) {
        auto const sim_start = std::chrono::high_resolution_clock::now();

        sim_.simulate(pool);

        std::cout << "Iteration " << i << " in "
                  << string::to_string(chrono::seconds_since(sim_start)) << " s" << std::endl;
    }

    current_frame_ = sim_frame;

    Volume_renderer renderer(density_.texture().dimensions_3(), 128);

    renderer.clear();

    for (uint32_t i = 0, len = sim_.num_tracers(); i < len; ++i) {
        float3 const p = sim_.world_to_texture_point(sim_.tracers()[i].position);

        renderer.splat(p, 1.f);
    }

    renderer.resolve(*static_cast<image::Float1*>(&density_.texture().image()));

    compile(pool);
}

bool Material::is_animated() const noexcept {
    return true;
}

}  // namespace procedural::fluid
