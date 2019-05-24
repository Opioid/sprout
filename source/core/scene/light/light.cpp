#include "light.hpp"
#include "base/math/aabb.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/vector3.inl"
#include "scene/entity/composed_transformation.hpp"
#include "scene/prop/prop.hpp"
#include "scene/scene.hpp"
#include "scene/scene_worker.hpp"

namespace scene::light {

Light::Light() = default;

static inline float3 prop_power(uint32_t prop, uint32_t part, AABB const& scene_bb,
                                Scene const& scene) noexcept {
    float const area = scene.prop_area(prop, part);

    float3 const radiance = scene.prop_material(prop, part)->average_radiance(area);

    if (scene.prop(prop)->shape()->is_finite()) {
        return area * radiance;
    } else {
        return squared_length(scene_bb.halfsize()) * area * radiance;
    }
}

static inline float3 volume_power(uint32_t prop, uint32_t part, Scene const& scene) noexcept {
    float const volume = scene.prop_volume(prop, part);

    float3 const radiance = scene.prop_material(prop, part)->average_radiance(volume);

    return volume * radiance;
}

float3 Light::power(AABB const& scene_bb, Scene const& scene) const noexcept {
    switch (type_) {
        case Type::Null:
            return float3(0.f);
        case Type::Prop:
            return prop_power(prop_, part_, scene_bb, scene);
        case Type::Prop_image:
            return prop_power(prop_, part_, scene_bb, scene);
        case Type::Volume:
            return volume_power(prop_, part_, scene);
        case Type::Volume_image:
            return volume_power(prop_, part_, scene);
    }

    return float3(0.);
}

void Light::prepare_sampling(uint32_t light_id, uint64_t time, Scene& scene,
                             thread::Pool& pool) const noexcept {
    switch (type_) {
        case Type::Null:
            break;
        case Type::Prop:
            scene.prop_prepare_sampling(prop_, part_, light_id, time, false, pool);
            break;
        case Type::Prop_image:
            scene.prop_prepare_sampling(prop_, part_, light_id, time, true, pool);
            break;
        case Type::Volume:
            scene.prop_prepare_sampling_volume(prop_, part_, light_id, time, false, pool);
            break;
        case Type::Volume_image:
            scene.prop_prepare_sampling_volume(prop_, part_, light_id, time, true, pool);
            break;
    }
}

bool Light::is_light(uint32_t id) noexcept {
    return 0xFFFFFFFF != id;
}

bool Light::is_area_light(uint32_t id) noexcept {
    return 0 == (id & Volume_light_mask);
}

uint32_t Light::strip_mask(uint32_t id) noexcept {
    return ~Volume_light_mask & id;
}

}  // namespace scene::light
