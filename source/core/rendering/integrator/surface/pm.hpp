#ifndef SU_RENDERING_INTEGRATOR_SURFACE_PM_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_PM_HPP

#include "sampler/sampler_random.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) PM final : public Integrator {
  public:
    struct Settings {
        uint32_t min_bounces;
        uint32_t max_bounces;

        bool photons_not_only_through_specular;
    };

    PM(rnd::Generator& rng, Settings const& settings, bool progressive) noexcept;

    ~PM() noexcept override final;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    float4 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept override final;

  private:
    sampler::Sampler& material_sampler(uint32_t bounce) noexcept;

    Settings const settings_;

    sampler::Random sampler_;

    sampler::Pool* sampler_pool_;

    static uint32_t constexpr Num_dedicated_samplers = 3;

    sampler::Sampler* material_samplers_[Num_dedicated_samplers];
};

class PM_pool final : public Typed_pool<PM> {
  public:
    PM_pool(uint32_t num_integrators, bool progressive, uint32_t min_bounces, uint32_t max_bounces,
            bool photons_only_through_specular) noexcept;

    Integrator* get(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    PM::Settings settings_;

    bool progressive_;
};

}  // namespace rendering::integrator::surface

#endif
