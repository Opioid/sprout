#ifndef SU_CORE_SCENE_SCENE_INL
#define SU_CORE_SCENE_SCENE_INL

#include "base/math/aabb.inl"
#include "entity/composed_transformation.inl"
#include "entity/keyframe.hpp"
#include "light/light.inl"
#include "prop/prop.inl"
#include "scene.hpp"
#include "scene_ray.hpp"

#include "base/debug/assert.hpp"

namespace scene {

inline bool Scene::has_volumes() const noexcept {
    return has_volumes_;
}

inline bool Scene::intersect(Ray& ray, Worker const& worker, prop::Intersection& intersection) const
    noexcept {
    return prop_bvh_.intersect(ray, worker, intersection);
}

inline bool Scene::intersect(Ray& ray, Worker const& worker, shape::Normals& normals) const
    noexcept {
    return prop_bvh_.intersect(ray, worker, normals);
}

inline bool Scene::intersect_volume(Ray& ray, Worker const& worker,
                                    prop::Intersection& intersection) const noexcept {
    return volume_bvh_.intersect_fast(ray, worker, intersection);
}

inline bool Scene::intersect_p(Ray const& ray, Worker const& worker) const noexcept {
    return prop_bvh_.intersect_p(ray, worker);
}

inline bool Scene::visibility(Ray const& ray, Filter filter, Worker const& worker, float& v) const
    noexcept {
    if (has_masked_material_) {
        return prop_bvh_.visibility(ray, filter, worker, v);
    }

    if (!prop_bvh_.intersect_p(ray, worker)) {
        v = 1.f;
        return true;
    }

    v = 0.f;
    return false;
}

inline bool Scene::thin_absorption(Ray const& ray, Filter filter, Worker const& worker,
                                   float3& ta) const noexcept {
    if (has_tinted_shadow_) {
        return prop_bvh_.thin_absorption(ray, filter, worker, ta);
    }

    if (float v; Scene::visibility(ray, filter, worker, v)) {
        ta = float3(v);
        return true;
    }

    return false;
}

inline std::vector<light::Light> const& Scene::lights() const noexcept {
    return lights_;
}

inline Scene::Transformation const& Scene::prop_world_transformation(uint32_t entity) const
    noexcept {
    return prop_world_transformations_[entity];
}

inline void Scene::prop_set_transformation(uint32_t                    entity,
                                           math::Transformation const& t) noexcept {
    uint32_t const f = prop_frames_[entity];

    entity::Keyframe& local_frame = keyframes_[f + num_interpolation_frames_];

    local_frame.transformation = t;
    local_frame.time           = scene::Static_time;
}

inline void Scene::prop_set_world_transformation(uint32_t                    entity,
                                                 math::Transformation const& t) noexcept {
    prop_world_transformations_[entity].set(t);
}

inline Scene::Transformation const& Scene::prop_transformation_at(
    uint32_t entity, uint64_t time, bool is_static, Transformation& transformation) const noexcept {
    if (is_static) {
        return prop_world_transformation(entity);
    }

    entity::Keyframe const* frames = &keyframes_[prop_frames_[entity]];

    for (uint32_t i = 0, len = num_interpolation_frames_ - 1; i < len; ++i) {
        auto const& a = frames[i];
        auto const& b = frames[i + 1];

        if ((time >= a.time) & (time < b.time)) {
            uint64_t const range = b.time - a.time;
            uint64_t const delta = time - a.time;

            float const t = float(delta) / float(range);

            transformation.set(lerp(a.transformation, b.transformation, t));

            break;
        }
    }

    return transformation;
}

inline math::Transformation const& Scene::prop_local_frame_0(uint32_t entity) const noexcept {
    entity::Keyframe const* frames = &keyframes_[prop_frames_[entity]];

    return frames[num_interpolation_frames_].transformation;
}

inline AABB const& Scene::prop_aabb(uint32_t entity) const noexcept {
    return prop_aabbs_[entity];
}

inline bool Scene::prop_aabb_intersect_p(uint32_t entity, Ray const& ray) const noexcept {
    return prop_aabbs_[entity].intersect_p(ray);
}

inline shape::Shape* Scene::prop_shape(uint32_t entity) const noexcept {
    return shape_resources_[props_[entity].shape()];
}

inline material::Material const* Scene::prop_material(uint32_t entity, uint32_t part) const
    noexcept {
    uint32_t const p = prop_parts_[entity] + part;
    return material_resources_[materials_[p]];
}

inline prop::Prop_topology const& Scene::prop_topology(uint32_t entity) const noexcept {
    return prop_topology_[entity];
}

inline image::texture::Texture const* Scene::texture(uint32_t id) const noexcept {
    return texture_resources_[id];
}

inline uint32_t Scene::prop_light_id(uint32_t entity, uint32_t part) const noexcept {
    uint32_t const p = prop_parts_[entity] + part;
    return light_ids_[p];
}

inline float Scene::light_area(uint32_t entity, uint32_t part) const noexcept {
    uint32_t const p = prop_parts_[entity] + part;

    uint32_t const light_id = light_ids_[p];

    if (prop::Null == light_id) {
        return 1.f;
    }

    return lights_[light_id].area();
}

inline float3 Scene::light_center(uint32_t light) const noexcept {
    return light_centers_[light];
}

inline uint32_t Scene::num_props() const noexcept {
    return uint32_t(props_.size());
}

inline prop::Prop const* Scene::prop(uint32_t index) const noexcept {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

inline prop::Prop* Scene::prop(uint32_t index) noexcept {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

inline prop::Prop* Scene::prop(std::string_view name) noexcept {
    auto e = named_props_.find(name);
    if (named_props_.end() == e) {
        return nullptr;
    }

    return &props_[e->second];
}

}  // namespace scene

#endif
