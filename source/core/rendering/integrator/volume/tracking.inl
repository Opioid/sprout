#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_INL
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_INL

#include "base/math/ray.inl"
#include "scene/scene.inl"
#include "scene/scene_ray.hpp"
#include "scene/scene_worker.inl"
#include "tracking.hpp"

namespace rendering::integrator::volume {

inline math::ray texture_space_ray(scene::Ray const& ray, uint32_t entity,
                                   scene::Worker const& worker) {
    scene::Scene::Transformation temp;
    auto const& transformation = worker.scene().prop_transformation_at(entity, ray.time, temp);

    float3 const local_origin = transformation.world_to_object_point(ray.origin);
    float3 const local_dir    = transformation.world_to_object_vector(ray.direction);

    auto const shape = worker.scene().prop_shape(entity);

    float3 const origin = shape->object_to_texture_point(local_origin);
    float3 const dir    = shape->object_to_texture_vector(local_dir);

    return math::ray(origin, dir, ray.min_t, ray.max_t);
}

}  // namespace rendering::integrator::volume

#endif
