#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_EMISSION_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_EMISSION_HPP

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Emission final : public Integrator {
  public:
    struct Settings {
        float step_size;
    };

    Emission(rnd::Generator& rng, Settings const& settings) noexcept;

    virtual void prepare(scene::Scene const& scene, uint32_t num_samples_per_pixel) noexcept final;

    virtual void start_pixel() noexcept final;

    virtual bool transmittance(Ray const& ray, Worker& worker,
                               float3& transmittance) noexcept final;

    virtual Event integrate(Ray& ray, Intersection& intersection, Filter filter, Worker& worker,
                            float3& li, float3& transmittance) noexcept final;

    Settings const settings_;
};

class Emission_pool final : public Typed_pool<Emission> {
  public:
    Emission_pool(uint32_t num_integrators, float step_size) noexcept;

    Integrator* get(uint32_t id, rnd::Generator& rng) const noexcept final;

  private:
    const Emission::Settings settings_;
};

}  // namespace rendering::integrator::volume

#endif
