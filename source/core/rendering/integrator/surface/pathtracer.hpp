#ifndef SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_HPP

#include "sampler/sampler_random.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Pathtracer final : public Integrator {
  public:
    struct Settings {
        uint32_t num_samples;
        uint32_t min_bounces;
        uint32_t max_bounces;

        bool avoid_caustics;
    };

    Pathtracer(Settings const& settings, bool progressive);

    ~Pathtracer() final;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) final;

    void start_pixel(RNG& rng) final;

    float4 li(Ray& ray, Intersection& isec, Worker& worker,
              Interface_stack const& initial_stack, AOV& aov) final;

  private:
    float4 integrate(Ray& ray, Intersection& isec, Worker& worker);

    sampler::Sampler& material_sampler(uint32_t bounce);

    Settings const settings_;

    sampler::Random sampler_;

    sampler::Pool* sampler_pool_;

    static uint32_t constexpr Num_dedicated_samplers = 3;

    sampler::Sampler* material_samplers_[Num_dedicated_samplers];
};

class Pathtracer_pool final : public Typed_pool<Pathtracer> {
  public:
    Pathtracer_pool(uint32_t num_integrators, bool progressive, uint32_t num_samples,
                    uint32_t min_bounces, uint32_t max_bounces, bool enable_caustics);

    Integrator* get(uint32_t id) const final;

  private:
    Pathtracer::Settings settings_;

    bool progressive_;
};

}  // namespace rendering::integrator::surface

#endif
