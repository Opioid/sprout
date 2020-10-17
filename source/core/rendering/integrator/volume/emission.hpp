#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_EMISSION_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_EMISSION_HPP

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Emission final : public Integrator {
  public:
    struct Settings {
        float step_size;
    };

    Emission(Settings const& settings);

    void prepare(scene::Scene const& scene, uint32_t num_samples_per_pixel) final;

    void start_pixel(rnd::Generator& rng) final;

    bool transmittance(Ray const& ray, Worker& worker, float3& transmittance) final;

    Event integrate(Ray& ray, Intersection& isec, Filter filter, Worker& worker, float3& li,
                    float3& transmittance) final;

    Settings const settings_;
};

class Emission_pool final : public Typed_pool<Emission> {
  public:
    Emission_pool(uint32_t num_integrators, float step_size);

    Integrator* get(uint32_t id) const final;

  private:
    const Emission::Settings settings_;
};

}  // namespace rendering::integrator::volume

#endif
