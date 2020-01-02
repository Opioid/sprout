#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP

#include "volume_integrator.hpp"

namespace scene::prop {
struct Interface;
}

namespace rendering::integrator::volume {

class alignas(64) Tracking_multi final : public Integrator {
  public:
    Tracking_multi(rnd::Generator& rng) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    bool transmittance(Ray const& ray, Worker& worker, float3& tr) noexcept override final;

    Event integrate(Ray& ray, Intersection& intersection, Filter filter, Worker& worker, float3& li,
                    float3& tr) noexcept override final;
};

class Tracking_multi_pool final : public Typed_pool<Tracking_multi> {
  public:
    Tracking_multi_pool(uint32_t num_integrators) noexcept;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;
};

}  // namespace rendering::integrator::volume

#endif
