#include "emission.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "rendering/integrator/integrator_helper.hpp"
#include "rendering/integrator/volume/volume_integrator.inl"
#include "rendering/rendering_worker.hpp"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_ray.inl"
#include "scene/shape/shape.hpp"
#include "tracking.hpp"

namespace rendering::integrator::volume {

Emission::Emission(Settings const& settings) : settings_(settings) {}

void Emission::prepare(uint32_t /*num_samples_per_pixel*/) {}

void Emission::start_pixel(rnd::Generator& /*rng*/) {}

bool Emission::transmittance(Ray const& ray, Worker& worker, float3& transmittance) {
    return Tracking::transmittance(ray, worker, transmittance);
}

Event Emission::integrate(Ray& /*ray*/, Intersection& /*isec*/, Filter /*filter*/,
                          Worker& /*worker*/, float3& /*li*/, float3& /*transmittance*/) {
    return Event::Pass;
}

Emission_pool::Emission_pool(uint32_t num_integrators, float step_size)
    : Typed_pool<Emission>(num_integrators), settings_{step_size} {}

Integrator* Emission_pool::get(uint32_t id) const {
    if (uint32_t const zero = 0;
        0 == std::memcmp(&zero, static_cast<void*>(&integrators_[id]), 4)) {
        return new (&integrators_[id]) Emission(settings_);
    }

    return &integrators_[id];
}

}  // namespace rendering::integrator::volume
