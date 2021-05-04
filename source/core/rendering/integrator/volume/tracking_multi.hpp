#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP

#include "volume_integrator.hpp"

namespace scene::prop {
struct Interface;
}

namespace rendering::integrator::volume {

class alignas(64) Tracking_multi final : public Integrator {
  public:
    Tracking_multi();

    void start_pixel(RNG& rng) final;

    bool transmittance(Ray const& ray, Filter filter, Worker& worker, float3& tr) final;

    Event integrate(Ray& ray, Intersection& isec, Filter filter, Worker& worker, float3& li,
                    float3& tr) final;
};

class Tracking_multi_pool final : public Typed_pool<Tracking_multi> {
  public:
    Tracking_multi_pool(uint32_t num_integrators);

    Integrator* create(uint32_t id, uint32_t max_samples_per_pixel) const final;
};

}  // namespace rendering::integrator::volume

#endif
