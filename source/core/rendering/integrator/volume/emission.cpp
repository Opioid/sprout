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

Emission::Emission(rnd::Generator& rng, Settings const& settings) noexcept
    : Integrator(rng), settings_(settings) {}

void Emission::prepare(scene::Scene const& /*scene*/, uint32_t /*num_samples_per_pixel*/) noexcept {
}

void Emission::start_pixel() noexcept {}

bool Emission::transmittance(Ray const& ray, Worker& worker, float3& transmittance) noexcept {
    return Tracking::transmittance(ray, rng_, worker, transmittance);
}

Event Emission::integrate(Ray& /*ray*/, Intersection& /*intersection*/, Filter /*filter*/,
                          Worker& /*worker*/, float3& /*li*/, float3& /*transmittance*/) noexcept {
    return Event::Pass;
}

Emission_pool::Emission_pool(uint32_t num_integrators, float step_size) noexcept
    : Typed_pool<Emission>(num_integrators), settings_{step_size} {}

Integrator* Emission_pool::create(uint32_t id, rnd::Generator& rng) const noexcept {
    return new (&integrators_[id]) Emission(rng, settings_);
}

}  // namespace rendering::integrator::volume
