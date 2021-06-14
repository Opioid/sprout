#ifndef SU_CORE_SCENE_SCENE_INL
#define SU_CORE_SCENE_SCENE_INL

#include "base/math/aabb.inl"
#include "entity/composed_transformation.inl"
#include "entity/keyframe.hpp"
#include "light/light.inl"
#include "prop/prop.inl"
#include "scene.hpp"
#include "scene_constants.hpp"
#include "scene_ray.hpp"
#include "shape/shape_intersection.hpp"

#include "base/debug/assert.hpp"

namespace scene {

static uint64_t constexpr Tick_duration = Units_per_second / 60;

inline AABB Scene::aabb() const {
    return prop_bvh_.aabb();
}

inline AABB Scene::caustic_aabb() const {
    return caustic_aabb_;
}

inline bool Scene::is_infinite() const {
    return !infinite_props_.empty() || !infinite_volumes_.empty();
}

inline bool Scene::has_volumes() const {
    return has_volumes_;
}

inline bool Scene::intersect(Ray& ray, Worker& worker, Interpolation ipo,
                             Intersection& isec) const {
    return prop_bvh_.intersect(ray, worker, ipo, isec);
}

inline bool Scene::intersect_shadow(Ray& ray, Worker& worker, Intersection& isec) const {
    return prop_bvh_.intersect_shadow(ray, worker, isec);
}

inline bool Scene::intersect_volume(Ray& ray, Worker& worker, Intersection& isec) const {
    return volume_bvh_.intersect(ray, worker, Interpolation::No_tangent_space, isec);
}

inline bool Scene::visibility(Ray const& ray, Filter filter, Worker& worker, float3& v) const {
    if (has_tinted_shadow_) {
        return prop_bvh_.visibility(ray, filter, worker, v);
    }

    bool const ip = prop_bvh_.intersect_p(ray, worker);

    v = float3(ip ? 0.f : 1.f);
    return !ip;
}

inline uint32_t Scene::num_interpolation_frames() const {
    return num_interpolation_frames_;
}

inline Scene::Transformation const& Scene::prop_world_transformation(uint32_t entity) const {
    return prop_world_transformations_[entity];
}

inline float3_p Scene::prop_world_position(uint32_t entity) const {
    return prop_world_positions_[entity];
}

inline void Scene::prop_set_transformation(uint32_t entity, math::Transformation const& t) {
    uint32_t const f = prop_frames_[entity];

    entity::Keyframe& local_frame = keyframes_[f + num_interpolation_frames_];

    local_frame.trafo = t;
}

inline void Scene::prop_set_world_transformation(uint32_t entity, math::Transformation const& t) {
    prop_world_transformations_[entity].prepare(t);
    prop_world_positions_[entity] = t.position;
}

inline Scene::Transformation const& Scene::prop_transformation_at(uint32_t entity, uint64_t time,
                                                                  Transformation& trafo) const {
    uint32_t const f = prop_frames_[entity];

    if (prop::Null == f) {
        return prop_world_transformation(entity);
    }

    return prop_animated_transformation_at(f, time, trafo);
}

inline Scene::Transformation const& Scene::prop_transformation_at(uint32_t entity, uint64_t time,
                                                                  bool            is_static,
                                                                  Transformation& trafo) const {
    if (is_static) {
        return prop_world_transformation(entity);
    }

    return prop_animated_transformation_at(prop_frames_[entity], time, trafo);
}

inline Scene::Frame Scene::frame_at(uint64_t time) const {
    uint64_t const i = (time - current_time_start_) / Tick_duration;

    uint64_t const a_time = current_time_start_ + i * Tick_duration;
    uint64_t const delta  = time - a_time;

    float const t = float(double(delta) / double(Tick_duration));

    return {uint32_t(i), t};
}

inline AABB const& Scene::prop_aabb(uint32_t entity) const {
    return prop_aabbs_[entity];
}

inline bool Scene::prop_aabb_intersect_p(uint32_t entity, Ray const& ray) const {
    return prop_aabbs_[entity].intersect_p(ray);
}

inline shape::Shape* Scene::prop_shape(uint32_t entity) const {
    return shape_resources_[props_[entity].shape()];
}

inline material::Material const* Scene::prop_material(uint32_t entity, uint32_t part) const {
    uint32_t const p = prop_parts_[entity] + part;
    return material_resources_[materials_[p]];
}

inline uint32_t Scene::prop_material_id(uint32_t entity, uint32_t part) const {
    uint32_t const p = prop_parts_[entity] + part;
    return materials_[p];
}

inline prop::Prop_topology const& Scene::prop_topology(uint32_t entity) const {
    return prop_topology_[entity];
}

inline uint32_t Scene::prop_light_id(uint32_t entity, uint32_t part) const {
    uint32_t const p = prop_parts_[entity] + part;
    return light_ids_[p];
}

inline float Scene::light_area(uint32_t entity, uint32_t part) const {
    uint32_t const p = prop_parts_[entity] + part;

    uint32_t const light_id = light_ids_[p];

    if (prop::Null == light_id) {
        return 1.f;
    }

    return lights_[light_id].area();
}

inline bool Scene::light_two_sided(uint32_t /*variant*/, uint32_t light) const {
    return lights_[light].is_two_sided();
}

inline float Scene::light_power(uint32_t /*variant*/, uint32_t light) const {
    return light_aabbs_[light].bounds[1][3];
}

inline AABB Scene::light_aabb(uint32_t light) const {
    return light_aabbs_[light];
}

inline float4 Scene::light_cone(uint32_t light) const {
    return light_cones_[light];
}

inline uint32_t Scene::num_props() const {
    return uint32_t(props_.size());
}

inline prop::Prop const* Scene::prop(uint32_t index) const {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

inline prop::Prop* Scene::prop(uint32_t index) {
    SOFT_ASSERT(index < props_.size());

    return &props_[index];
}

inline shape::Shape const* Scene::shape(uint32_t index) const {
    SOFT_ASSERT(index < shape_resources_.size());

    return shape_resources_[index];
}

inline material::Material const* Scene::material(uint32_t index) const {
    SOFT_ASSERT(index < material_resources_.size());

    return material_resources_[index];
}

inline image::Image const* Scene::image(uint32_t id) const {
    return image_resources_[id];
}

inline uint32_t Scene::num_lights() const {
    return uint32_t(lights_.size());
}

inline light::Light const& Scene::light(uint32_t id) const {
    return lights_[id];
}

}  // namespace scene

#endif
