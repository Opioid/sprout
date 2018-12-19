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
#include "fluid_simulation.hpp"
#include "volume_renderer.hpp"

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

    Volume_renderer renderer(dimensions, 1);

    auto target = std::make_shared<Float1>(Image::Description(Image::Type::Float1, dimensions));

    rnd::Generator rng(0, 0);

    for (uint32_t i = 0; i < 4096; ++i) {
        float3 const p(rng.random_float(), rng.random_float(), rng.random_float());

        float3 const c = p * float3(dimensions);

        renderer.splat(c, 1.f);
    }

    renderer.resolve(*target);

    auto texture = std::make_shared<texture::Float1>(target);

    auto material = std::make_shared<volumetric::Grid>(Sampler_settings(),
                                                       Texture_adapter(texture));

    material->set_attenuation(float3(0.8f), float3(0.5f), 0.001f);
    material->set_emission(float3(0.f));
    material->set_anisotropy(0.f);

    material->compile(manager.thread_pool());

    prop::Prop* prop = scene.create_prop(scene_loader_->cube(), {material});

    prop->set_visibility(true, true, true);

    return prop;
}

}  // namespace procedural::fluid
