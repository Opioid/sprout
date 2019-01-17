#include "flame_provider.hpp"
#include "base/math/vector3.inl"
#include "core/resource/resource_manager.hpp"
#include "core/scene/material/volumetric/volumetric_homogeneous.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"

namespace procedural::flame {

using namespace scene;
using namespace scene::material;

static Provider provider;

void init(scene::Loader& loader) {
    provider.set_scene_loader(loader);

    loader.register_extension_provider("Flame", &provider);
}

Provider::~Provider() noexcept {}

void Provider::set_scene_loader(Loader& loader) noexcept {
    scene_loader_ = &loader;
}

entity::Entity* Provider::create_extension(json::Value const& /*extension_value*/, Scene& scene,
                                           resource::Manager& manager) noexcept {
    //    Flame* flame = new Flame();

    //    return flame;

    //   auto texture = std::make_shared<texture::Float1>(target);

    auto material = new volumetric::Homogeneous(Sampler_settings());

    //    auto material = std::make_shared<volumetric::Homogeneous>(Sampler_settings());

    material->set_attenuation(float3(0.8f), float3(0.f), 0.1f);
    material->set_emission(float3(0.3f, 1.f, 0.5f));
    material->set_anisotropy(0.f);

    material->compile(manager.thread_pool());

    prop::Prop* prop = scene.create_prop(scene_loader_->cube(), {material});

    prop->set_visibility(true, true, true);

    scene_loader_->create_light(prop, scene);

    return prop;
}

}  // namespace procedural::flame
