#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DL
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DL

#include "sampler/sampler_random.hpp"
#include "scene/material/sampler_settings.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Pathtracer_DL final : public Integrator {
  public:
    struct Settings {
        uint32_t num_samples;
        uint32_t min_bounces;
        uint32_t max_bounces;

        Light_sampling light_sampling;

        bool avoid_caustics;
    };

    Pathtracer_DL(Settings const& settings, bool progressive);

    ~Pathtracer_DL() final;

    void prepare(uint32_t num_samples_per_pixel) final;

    void start_pixel(RNG& rng) final;

    float4 li(Ray& ray, Intersection& isec, Worker& worker, Interface_stack const& initial_stack,
              AOV* aov) final;

  private:
    float3 direct_light(Ray const& ray, Intersection const& isec, Material_sample const& mat_sample,
                        Filter filter, Worker& worker);

    sampler::Sampler& material_sampler(uint32_t bounce);

    sampler::Sampler& light_sampler(uint32_t bounce);

    bool splitting(uint32_t bounce) const;

    Settings const settings_;

    sampler::Random sampler_;

    sampler::Pool* sampler_pool_;

    static uint32_t constexpr Num_dedicated_samplers = 3;
};

class Pathtracer_DL_pool final : public Typed_pool<Pathtracer_DL> {
  public:
    Pathtracer_DL_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples,
                       uint32_t min_bounces, uint32_t max_bounces, Light_sampling light_sampling,
                       bool enable_caustics);

    Integrator* get(uint32_t id) const final;

  private:
    Pathtracer_DL::Settings settings_;

    bool progressive_;
};

}  // namespace rendering::integrator::surface

#endif
