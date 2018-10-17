#ifndef SU_CORE_RENDERING_INTEGRATRO_SURFACE_AO_HPP
#define SU_CORE_RENDERING_INTEGRATRO_SURFACE_AO_HPP

#include "sampler/sampler_golden_ratio.hpp"
#include "surface_integrator.hpp"
// #include "sampler/sampler_halton.hpp"
// #include "sampler/sampler_ld.hpp"
// #include "sampler/sampler_scrambled_hammersley.hpp"
// #include "sampler/sampler_sobol.hpp"
#include "sampler/sampler_random.hpp"

namespace rendering::integrator::surface {

class alignas(64) AO final : public Integrator {
  public:
    struct Settings {
        uint32_t num_samples;
        float    radius;
    };

    AO(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    float3 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    const Settings settings_;

    sampler::Golden_ratio sampler_;
};

class AO_factory final : public Factory {
  public:
    AO_factory(take::Settings const& settings, uint32_t num_integrators, uint32_t num_samples,
               float radius) noexcept;

    ~AO_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    AO* integrators_;

    AO::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
