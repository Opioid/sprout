#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DL1
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DL1

#include "sampler/sampler_random.hpp"
#include "scene/material/sampler_settings.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Pathtracer_DL final : public Integrator {
  public:
    struct Settings {
        uint32_t min_bounces;
        uint32_t max_bounces;

        uint32_t num_light_samples;

        bool avoid_caustics;
    };

    Pathtracer_DL(rnd::Generator& rng, Settings const& settings, bool progressive) noexcept;

    ~Pathtracer_DL() noexcept;

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

    sampler::Pool* sampler_pool_;

    static uint32_t constexpr Num_dedicated_samplers = 3;

    sampler::Sampler* material_samplers_[Num_dedicated_samplers];

    sampler::Sampler* light_samplers_[Num_dedicated_samplers];
};

class Pathtracer_DL_pool final : public Typed_pool<Pathtracer_DL> {
  public:
    Pathtracer_DL_pool(uint32_t num_integrators, bool progressive, uint32_t min_bounces,
                       uint32_t max_bounces, uint32_t num_light_samples,
                       bool enable_caustics) noexcept;

    Integrator* get(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Pathtracer_DL::Settings settings_;

    bool progressive_;
};

}  // namespace rendering::integrator::surface

#endif
