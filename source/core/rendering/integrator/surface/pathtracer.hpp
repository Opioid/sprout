#ifndef SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_HPP

#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Pathtracer final : public Integrator {
  public:
    struct Settings {
        uint32_t num_samples;
        uint32_t min_bounces;
        uint32_t max_bounces;
        float    path_continuation_probability;

        bool avoid_caustics;
    };

    Pathtracer(rnd::Generator& rng, take::Settings const& take_settings,
               Settings const& settings) noexcept;

    ~Pathtracer() noexcept override final;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void resume_pixel(uint32_t sample, rnd::Generator& scramble) noexcept override final;

    float3 li(Ray& ray, Intersection& intersection, Worker& worker, Interface_stack const& initial_stack) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    float3 integrate(Ray& ray, Intersection& intersection, Worker& worker) noexcept;

    sampler::Sampler& material_sampler(uint32_t bounce) noexcept;

    const Settings settings_;

    sampler::Random sampler_;

    static uint32_t constexpr Num_material_samplers = 3;
    sampler::Golden_ratio material_samplers_[Num_material_samplers];
};

class Pathtracer_factory final : public Factory {
  public:
    Pathtracer_factory(take::Settings const& take_settings, uint32_t num_integrators,
                       uint32_t num_samples, uint32_t min_bounces, uint32_t max_bounces,
                       float path_termination_probability, bool enable_caustics) noexcept;

    ~Pathtracer_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Pathtracer* integrators_;

    Pathtracer::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
