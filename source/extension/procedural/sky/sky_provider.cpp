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

void init(scene::Loader& loader) {
    provider.set_scene_loader(loader);

    loader.register_extension_provider("Sky", &provider);
}

Provider::~Provider() = default;

void Provider::set_scene_loader(Loader& loader) {
    scene_loader_ = &loader;
}

uint32_t Provider::create_extension(json::Value const& value, Scene& scene, Resources& resources) {
    Sky* sky = new Sky;

    static bool constexpr Bake = true;

    static char constexpr Sky_name[] = "proc:sky";
    static char constexpr Sun_name[] = "proc:sun";

    uint32_t sky_mat;

    if (auto const resource = resources.get<material::Material>(Sky_name); resource.ptr) {
        sky_mat = resource.id;

        static_cast<Material*>(resource.ptr)->set_sky(sky);
    } else {
        if (Bake) {
            sky_mat = resources.store<material::Material>(Sky_name,
                                                          new Sky_baked_material(sky, resources));
        } else {
            sky_mat = resources.store<material::Material>(Sky_name, new Sky_material(sky));
        }
    }

    uint32_t sun_mat;

    if (auto const resource = resources.get<material::Material>(Sun_name); resource.ptr) {
        sun_mat = resource.id;

        static_cast<Material*>(resource.ptr)->set_sky(sky);
    } else {
        if (Bake) {
            sun_mat = resources.store<material::Material>(Sun_name, new Sun_baked_material(sky));
        } else {
            sun_mat = resources.store<material::Material>(Sun_name, new Sun_material(sky));
        }
    }

    uint32_t const sky_prop = scene.create_prop(scene_loader_->canopy(), &sky_mat);
    uint32_t const sun_prop = scene.create_prop(scene_loader_->distant_sphere(), &sun_mat);

    sky->init(sky_prop, sun_prop, scene);

    if (auto const p = value.FindMember("parameters"); value.MemberEnd() != p) {
        sky->set_parameters(p->value, scene);
    } else {
        sky->update(scene);
    }

    scene.create_light(sky_prop);
    scene.create_light(sun_prop);

    return scene.create_extension(sky);
}

}  // namespace procedural::sky
