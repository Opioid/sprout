#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP

#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Tracking_multi final : public Integrator {
  public:
    Tracking_multi(rnd::Generator& rng, take::Settings const& take_settings) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void resume_pixel(uint32_t sample, rnd::Generator& scramble) noexcept override final;

    bool transmittance(Ray const& ray, Worker& worker,
                       float3& transmittance) noexcept override final;

    bool integrate(Ray& ray, Intersection& intersection, Sampler_filter filter, Worker& worker,
                   float3& li, float3& transmittance) noexcept override final;

    size_t num_bytes() const noexcept override final;
};

class Tracking_multi_factory final : public Factory {
  public:
    Tracking_multi_factory(take::Settings const& take_settings, uint32_t num_integrators) noexcept;

    ~Tracking_multi_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Tracking_multi* integrators_;
};

}  // namespace rendering::integrator::volume

#endif
