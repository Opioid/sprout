#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DLDL
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DLDL

#include "base/math/vector3.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "scene/material/sampler_settings.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Pathtracer_DLDL final : public Integrator {
  public:
    struct Settings {
        uint32_t num_samples;
        uint32_t min_bounces;
        uint32_t max_bounces;
    };

    Pathtracer_DLDL(rnd::Generator& rng, take::Settings const& take_settings,
                    Settings const& settings) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    float4 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept override final;

  private:
    float4 integrate(Ray& ray, Intersection& intersection, float3 const& wi,
                     Worker& worker) noexcept;

    float3 direct_light(Ray const& ray, Intersection const& intersection, float3 const& wi,
                        Material_sample const& material_sample, bool evaluate_back, Filter filter,
                        Worker& worker) noexcept;

    sampler::Sampler& material_sampler(uint32_t bounce) noexcept;
    sampler::Sampler& light_sampler(uint32_t bounce) noexcept;

    Settings const settings_;

    sampler::Random sampler_;

    float3 wi_;
};

class Pathtracer_DLDL_factory final : public Factory {
  public:
    Pathtracer_DLDL_factory(take::Settings const& take_settings, uint32_t num_integrators,
                            uint32_t num_samples, uint32_t min_bounces,
                            uint32_t max_bounces) noexcept;

    ~Pathtracer_DLDL_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Pathtracer_DLDL* integrators_;

    Pathtracer_DLDL::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
