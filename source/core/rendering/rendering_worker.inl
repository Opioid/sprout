#ifndef SU_CORE_RENDERING_WORKER_INL
#define SU_CORE_RENDERING_WORKER_INL

#include "rendering/integrator/particle/lighttracer.hpp"
#include "rendering/integrator/volume/volume_integrator.hpp"
#include "rendering_worker.hpp"

namespace rendering {

inline void Worker::particle_li(uint32_t frame, Interface_stack const& interface_stack) {
    lighttracer_->li(frame, *this, interface_stack);
}

inline Event Worker::volume(Ray& ray, Intersection& intersection, Filter filter, float3& li,
                            float3& tr) {
    return volume_integrator_->integrate(ray, intersection, filter, *this, li, tr);
}

}  // namespace rendering

#endif
