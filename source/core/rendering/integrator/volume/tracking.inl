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
    auto const& trafo = worker.scene().prop_transformation_at(entity, ray.time, temp);

    float3 const local_origin = trafo.world_to_object_point(ray.origin);
    float3 const local_dir    = trafo.world_to_object_vector(ray.direction);

    auto const shape = worker.scene().prop_shape(entity);

    AABB const aabb = shape->aabb();

    float3 const iextent = 1.f / aabb.extent();
    float3 const origin  = (local_origin - aabb.bounds[0]) * iextent;
    float3 const dir     = local_dir * iextent;

    return math::ray(origin, dir, ray.min_t(), ray.max_t());
}

}  // namespace rendering::integrator::volume

#endif
