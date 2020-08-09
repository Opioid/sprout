#ifndef SU_CORE_RENDERING_WORKER_INL
#define SU_CORE_RENDERING_WORKER_INL

#include "rendering/integrator/volume/volume_integrator.hpp"
#include "rendering_worker.hpp"
#include "scene/scene_worker.inl"

namespace rendering {

inline Event Worker::volume(Ray& ray, Intersection& intersection, Filter filter, float3& li,
                            float3& tr) {
    return volume_integrator_->integrate(ray, intersection, filter, *this, li, tr);
}

}  // namespace rendering

#endif
