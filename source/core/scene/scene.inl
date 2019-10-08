#ifndef SU_CORE_SCENE_SCENE_INL
#define SU_CORE_SCENE_SCENE_INL

#include "base/math/aabb.inl"
#include "entity/composed_transformation.inl"
#include "light/light.inl"
#include "prop/prop.inl"
#include "scene.hpp"
#include "scene_ray.hpp"

#include "base/debug/assert.hpp"

namespace scene {

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

inline void Scene::prop_set_world_transformation(uint32_t                    entity,
                                                 math::Transformation const& t) noexcept {
    prop_world_transformations_[entity].set(t);
}

inline Scene::Transformation const& Scene::prop_transformation_at(
    uint32_t entity, uint64_t time, bool is_static, Transformation& transformation) const noexcept {
    if (is_static) {
        return prop_world_transformation(entity);
    }

    prop_animated_transformation_at(entity, time, transformation);

    return transformation;
}

inline math::Transformation const& Scene::prop_local_frame_0(uint32_t entity) const noexcept {
    entity::Keyframe const* frames = &keyframes_[prop_frames_[entity]];

    return frames[num_interpolation_frames_].transformation;
}

inline bool Scene::prop_aabb_intersect_p(uint32_t entity, Ray const& ray) const noexcept {
    return prop_aabbs_[entity].intersect_p(ray);
}

inline shape::Shape* Scene::prop_shape(uint32_t entity) const noexcept {
    return shape_resources_[props_[entity].shape()];
}

inline material::Material const* Scene::prop_material(uint32_t entity, uint32_t part) const
    noexcept {
    return material_resources_[prop_materials_[entity].materials[part]];
}

inline prop::Prop_topology const& Scene::prop_topology(uint32_t entity) const noexcept {
    return prop_topology_[entity];
}

inline uint32_t Scene::prop_light_id(uint32_t entity, uint32_t part) const noexcept {
    return prop_materials_[entity].light_ids[part];
}

inline float Scene::light_area(uint32_t entity, uint32_t part) const noexcept {
    uint32_t const light_id = prop_materials_[entity].light_ids[part];

    if (prop::Null == light_id) {
        return 1.f;
    }

    return lights_[light_id].area();
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
