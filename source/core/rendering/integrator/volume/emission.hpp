#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_EMISSION_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_EMISSION_HPP

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Emission final : public Integrator {
  public:
    struct Settings {
        float step_size;
    };

    Emission(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings) noexcept;

    virtual void prepare(scene::Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) noexcept override final;

    virtual bool transmittance(Ray const& ray, Worker& worker,
                               float3& transmittance) noexcept override final;

    virtual bool integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
                           Worker& worker, float3& li, float3& transmittance) noexcept override final;

    virtual size_t num_bytes() const noexcept override final;

    const Settings settings_;
};

class Emission_factory final : public Factory {
  public:
    Emission_factory(take::Settings const& settings, uint32_t num_integrators, float step_size) noexcept;

    virtual ~Emission_factory() noexcept override final;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Emission* integrators_;

    const Emission::Settings settings_;
};

}  // namespace rendering::integrator::volume

#endif
