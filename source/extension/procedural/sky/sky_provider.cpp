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
using Light_type = scene::light::Light::Type;

static Provider provider;

void init(scene::Loader& loader, material::Provider& material_provider) {
    provider.set_scene_loader(loader);
    provider.set_material_provider(material_provider);

    loader.register_extension_provider("Sky", &provider);
}

Provider::~Provider() = default;

void Provider::set_scene_loader(Loader& loader) {
    scene_loader_ = &loader;
}

void Provider::set_material_provider(material::Provider& material_provider) {
    material_provider_ = &material_provider;
}

uint32_t Provider::create_extension(json::Value const& extension_value, Scene& scene,
                                    Resources& resources) {
    Sky* sky = new Sky;

    uint32_t const sky_entity = scene.create_extension(sky);

    static bool constexpr Bake = true;

    static char constexpr Sky_name[] = "proc:sky";
    static char constexpr Sun_name[] = "proc:sun";

    uint32_t sky_material_id;

    if (auto const resource = resources.get<material::Material>(Sky_name); resource.ptr) {
        sky_material_id = resource.id;

        static_cast<Material*>(resource.ptr)->set_sky(sky);
    } else {
        if (Bake) {
            sky_material_id = resources.store<material::Material>(Sky_name,
                                                                  new Sky_baked_material(sky));
        } else {
            sky_material_id = resources.store<material::Material>(Sky_name, new Sky_material(sky));
        }
    }

    uint32_t sun_material_id;

    if (auto const resource = resources.get<material::Material>(Sun_name); resource.ptr) {
        sun_material_id = resource.id;

        static_cast<Material*>(resource.ptr)->set_sky(sky);
    } else {
        if (Bake) {
            sun_material_id = resources.store<material::Material>(Sun_name,
                                                                  new Sun_baked_material(sky));
        } else {
            sun_material_id = resources.store<material::Material>(Sun_name, new Sun_material(sky));
        }
    }

    uint32_t const sky_prop = scene.create_prop(scene_loader_->canopy(), &sky_material_id);
    uint32_t const sun_prop = scene.create_prop(scene_loader_->distant_sphere(), &sun_material_id);

    sky->init(sky_prop, sun_prop, scene);

    if (auto const p = extension_value.FindMember("parameters"); extension_value.MemberEnd() != p) {
        sky->set_parameters(p->value, scene);
    } else {
        sky->update(scene);
    }

    scene.create_light(sky_prop);
    scene.create_light(sun_prop);

    return sky_entity;
}

}  // namespace procedural::sky
