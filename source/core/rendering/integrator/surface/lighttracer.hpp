#ifndef SU_RENDERING_INTEGRATOR_SURFACE_LIGHTTRACER_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_LIGHTTRACER_HPP

#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Lighttracer final : public Integrator {
  public:
    struct Settings {
        uint32_t min_bounces;
        uint32_t max_bounces;
    };

    Lighttracer(rnd::Generator& rng, take::Settings const& take_settings,
                Settings const& settings) noexcept;

    ~Lighttracer() noexcept override final;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    float3 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    bool generate_light_ray(uint64_t time, Worker& worker, Ray& ray, float3& radiance) noexcept;

    float3 direct_light(Ray const& ray, Intersection const& intersection,
                        Material_sample const& material_sample, Filter filter,
                        Worker& worker) noexcept;

    sampler::Sampler& material_sampler(uint32_t bounce) noexcept;

    Settings const settings_;

    sampler::Random sampler_;

    static uint32_t constexpr Num_material_samplers = 3;
    sampler::Golden_ratio material_samplers_[Num_material_samplers];
};

class Lighttracer_factory final : public Factory {
  public:
    Lighttracer_factory(take::Settings const& take_settings, uint32_t num_integrators,
                        uint32_t min_bounces, uint32_t max_bounces) noexcept;

    ~Lighttracer_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

    uint32_t max_sample_depth() const noexcept override final;

  private:
    Lighttracer* integrators_;

    Lighttracer::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
