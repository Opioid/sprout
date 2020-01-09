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

    AO(rnd::Generator& rng, Settings const& settings, bool progressive) noexcept;

    ~AO() noexcept override final;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    float4 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept override final;

  private:
    Settings const settings_;

    sampler::Sampler* sampler_;
};

class AO_pool final : public Typed_pool<AO> {
  public:
    AO_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples, float radius) noexcept;

    Integrator* get(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    AO::Settings settings_;

    bool progressive_;
};

}  // namespace rendering::integrator::surface

#endif
