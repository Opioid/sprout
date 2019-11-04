#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DLDL
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DLDL

#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "scene/material/sampler_settings.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Pathtracer_DLDL final : public Integrator {
  public:
    struct Settings {
        uint32_t min_bounces;
        uint32_t max_bounces;

        uint32_t num_light_samples;

        bool avoid_caustics;
    };

    Pathtracer_DLDL(rnd::Generator& rng, take::Settings const& take_settings,
                    Settings const& settings) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    float4 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept override final;

  private:
    float3 direct_light(Ray const& ray, Intersection const& intersection,
                        Material_sample const& material_sample, bool evaluate_back, Filter filter,
                        Worker& worker) noexcept;

    sampler::Sampler& material_sampler(uint32_t bounce) noexcept;
    sampler::Sampler& light_sampler(uint32_t bounce) noexcept;

    Settings const settings_;

    sampler::Random sampler_;

    static uint32_t constexpr Num_material_samplers = 3;
    sampler::Golden_ratio material_samplers_[Num_material_samplers];

    static uint32_t constexpr Num_light_samplers = 3;
    sampler::Golden_ratio light_samplers_[Num_light_samplers];
};

class Pathtracer_DLDL_factory final : public Factory {
  public:
    Pathtracer_DLDL_factory(take::Settings const& take_settings, uint32_t num_integrators,
                            uint32_t min_bounces, uint32_t max_bounces, uint32_t num_light_samples,
                            bool enable_caustics) noexcept;

    ~Pathtracer_DLDL_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Pathtracer_DLDL* integrators_;

    Pathtracer_DLDL::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
