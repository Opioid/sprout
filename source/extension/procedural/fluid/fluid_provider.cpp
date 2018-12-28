#include "fluid_provider.hpp"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "core/image/texture/texture_adapter.inl"
#include "core/image/texture/texture_float_1.hpp"
#include "core/image/typed_image.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/scene/material/volumetric/volumetric_grid.hpp"
#include "core/scene/material/volumetric/volumetric_homogeneous.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "fluid_particle.hpp"
#include "fluid_simulation.hpp"
#include "fluid_vorton.hpp"
#include "volume_renderer.hpp"
#include "base/chrono/chrono.hpp"
#include "base/string/string.hpp"

#include <iostream>

namespace procedural::fluid {

using namespace scene;
using namespace scene::material;

static Provider provider;

void init(scene::Loader& loader) {
    provider.set_scene_loader(loader);

    loader.register_extension_provider("Fluid", &provider);
}

Provider::~Provider() noexcept {}

void Provider::set_scene_loader(Loader& loader) noexcept {
    scene_loader_ = &loader;
}

entity::Entity* Provider::create_extension(json::Value const& /*extension_value*/, Scene& scene,
                                           resource::Manager& manager) noexcept {
    using namespace image;

    //    auto material = std::make_shared<volumetric::Homogeneous>(Sampler_settings());

    int3 dimensions(64);

    Simulation sim(dimensions);

	std::cout << "Simulating..." << std::endl;

	    rnd::Generator rng(0, 0);

		for (uint32_t i = 0, len = sim.num_vortons(); i < len; ++i) {
            Vorton& v = sim.vortons()[i];

			v.radius = 0.01f;

            float3 const p(rng.random_float(), rng.random_float(), rng.random_float());

            v.position = 0.2f * (2.f * p - 1.f);

			
			float3 const vorticity = 2.f * float3(rng.random_float(), rng.random_float(),
                                                  rng.random_float()) - 1.f;

            v.vorticity = 16.f * normalize(vorticity);
        }

        for (uint32_t i = 0, len = sim.num_tracers(); i < len; ++i) {
            float3 const p(rng.random_float(), rng.random_float(), rng.random_float());

            sim.tracers()[i].position = (0.05f + 0.01f * rng.random_float()) * normalize(2.f * p - 1.f);
        }


	for (uint32_t i = 0; i < 3; ++i) {
        auto const start = std::chrono::high_resolution_clock::now();

        sim.simulate(manager.thread_pool());

		std::cout << "Iteration " << i << " in "
                  << string::to_string(chrono::seconds_since(start)) << " s" << std::endl;
	}

	int3 const viz_dimensions(320);

    Volume_renderer renderer(viz_dimensions, 128);

	renderer.clear();

    auto target = std::make_shared<Float1>(
            Image::Description(Image::Type::Float1, viz_dimensions));

	for (uint32_t i = 0, len = sim.num_tracers(); i < len; ++i) {
        float3 const p = sim.world_to_texture_point(sim.tracers()[i].position);

        renderer.splat(p, 1.f);
    }

    renderer.resolve(*target);

    auto texture = std::make_shared<texture::Float1>(target);

    auto material = std::make_shared<volumetric::Grid>(Sampler_settings(),
                                                       Texture_adapter(texture));

    material->set_attenuation(float3(0.8f), float3(0.5f), 0.0005f);
    material->set_emission(float3(0.f));
    material->set_anisotropy(0.f);

    material->compile(manager.thread_pool());

    prop::Prop* prop = scene.create_prop(scene_loader_->cube(), {material});

    prop->set_visibility(true, true, true);

    return prop;
}

}  // namespace procedural::fluid
