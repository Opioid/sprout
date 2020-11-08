#ifndef SU_CORE_RENDERING_INTEGRATRO_SURFACE_AO_HPP
#define SU_CORE_RENDERING_INTEGRATRO_SURFACE_AO_HPP

#include "surface_integrator.hpp"

namespace sampler {
class Sampler;
}

namespace rendering::integrator::surface {

class alignas(64) AO final : public Integrator {
  public:
    struct Settings {
        uint32_t num_samples;
        float    radius;
    };

    AO(Settings const& settings, bool progressive);

    ~AO() final;

    void prepare(Scene const& scene, uint32_t max_samples_per_pixel) final;

    void start_pixel(RNG& rng, uint32_t num_samples) final;

    float4 li(Ray& ray, Intersection& isec, Worker& worker, Interface_stack const& initial_stack,
              AOV* aov) final;

  private:
    Settings const settings_;

    sampler::Sampler* sampler_;
};

class AO_pool final : public Typed_pool<AO> {
  public:
    AO_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples, float radius);

    Integrator* get(uint32_t id) const final;

  private:
    AO::Settings settings_;

    bool progressive_;
};

}  // namespace rendering::integrator::surface

#endif
