#include "fluid_provider.hpp"
#include "base/math/vector3.inl"
#include "core/resource/resource_manager.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "fluid_material.hpp"

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
                                           resource::Manager& /*manager*/) noexcept {
    auto material = std::make_shared<Material>(Sampler_settings(Sampler_settings::Filter::Linear,
                                                                Sampler_settings::Address::Clamp,
                                                                Sampler_settings::Address::Clamp));

    material->set_attenuation(0.5f, 0.0004f);
    material->set_emission(float3(0.f));
    material->set_anisotropy(0.f);

    scene.add_material(material);

    prop::Prop* prop = scene.create_prop(scene_loader_->cube(), {material});

    prop->set_visibility(true, true, true);

    material->set_prop(prop);

    return prop;
}

}  // namespace procedural::fluid
