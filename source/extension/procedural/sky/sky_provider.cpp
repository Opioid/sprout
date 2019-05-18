#include "sky_provider.hpp"
#include "base/json/json.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "core/resource/resource_manager.inl"
#include "core/scene/material/light/light_constant.hpp"
#include "core/scene/material/material_provider.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_loader.hpp"
#include "sky.hpp"
#include "sky_material.hpp"
#include "sun_material.hpp"

namespace procedural::sky {

using namespace scene;

static Provider provider;

void init(scene::Loader& loader, material::Provider& material_provider) noexcept {
    provider.set_scene_loader(loader);
    provider.set_material_provider(material_provider);

    loader.register_extension_provider("Sky", &provider);
}

Provider::~Provider() noexcept {}

void Provider::set_scene_loader(Loader& loader) noexcept {
    scene_loader_ = &loader;
}

void Provider::set_material_provider(material::Provider& material_provider) noexcept {
    material_provider_ = &material_provider;
}

prop::Prop_ref Provider::create_extension(json::Value const& extension_value,
                                          std::string const& name, Scene& scene,
                                          resource::Manager& manager) noexcept {
    Sky* sky = new Sky;

    Prop_ref sky_entity = scene.create_extension(sky /*, name*/);

    static bool constexpr bake = true;

    Material* sky_material;
    Material* sun_material;

    if (bake) {
        sky_material = new Sky_baked_material(*sky);
        sun_material = new Sun_baked_material(*sky);
    } else {
        sky_material = new Sky_material(*sky);
        sun_material = new Sun_material(*sky);
    }

    manager.store<material::Material>("proc:sky", sky_material);

    manager.store<material::Material>("proc:sun", sun_material);

    Scene::Prop_ref sky_prop = scene.create_prop(scene_loader_->canopy(), {sky_material});
    sky_prop.ref->allocate_frames(1, 1);

    Scene::Prop_ref sun_prop = scene.create_prop(scene_loader_->celestial_disk(), {sun_material});
    sun_prop.ref->allocate_frames(1, 1);

    sky->init(sky_prop.id, sun_prop.id, scene);

    if (auto const p = extension_value.FindMember("parameters"); extension_value.MemberEnd() != p) {
        sky->set_parameters(p->value, scene);
    } else {
        sky->update(scene);
    }

    if (bake) {
        scene.create_prop_image_light(sky_prop.id, 0);
    } else {
        scene.create_prop_light(sky_prop.id, 0);
    }

    scene.create_prop_light(sun_prop.id, 0);

    return sky_entity;
}

}  // namespace procedural::sky
