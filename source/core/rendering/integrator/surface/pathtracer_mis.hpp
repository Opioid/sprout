#ifndef SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_NG_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_NG_HPP

#include "base/math/vector3.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "surface_integrator.hpp"

namespace rendering::integrator::surface {

class alignas(64) Pathtracer_MIS final : public Integrator {
  public:
    struct Settings {
        uint32_t num_samples;
        uint32_t min_bounces;
        uint32_t max_bounces;

        Light_sampling light_sampling;

        bool avoid_caustics;
    };

    Pathtracer_MIS(rnd::Generator& rng, take::Settings const& take_settings,
                   Settings const& settings) noexcept;

    ~Pathtracer_MIS() override final;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    float3 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    struct Result {
        float3 li;
        float3 photon_li;
        bool   split_photon;
    };

    Result integrate(Ray& ray, Intersection& intersection, Worker& worker,
                     bool integrate_photons) noexcept;

    float3 sample_lights(Ray const& ray, float ray_offset, Intersection& intersection,
                         const Material_sample& material_sample, bool evaluate_back, Filter filter,
                         Worker& worker) noexcept;

    float3 evaluate_light(const Light& light, float light_weight, Ray const& history,
                          float ray_offset, uint32_t sampler_dimension, bool evaluate_back,
                          Intersection const& intersection, const Material_sample& material_sample,
                          Filter filter, Worker& worker) noexcept;

    float3 evaluate_light(Ray const& ray, Intersection const& intersection,
                          Bxdf_sample sample_result, bool treat_as_singular, bool is_translucent,
                          Filter filter, Worker& worker, bool& pure_emissive) noexcept;

    float3 evaluate_light_volume(float3 const& vli, Ray const& ray,
                                 Intersection const& intersection, float bxdf_pdf,
                                 bool treat_as_singular, bool is_translucent, Worker& worker) const
        noexcept;

    sampler::Sampler& material_sampler(uint32_t bounce) noexcept;
    sampler::Sampler& light_sampler(uint32_t bounce) noexcept;

    const Settings settings_;

    sampler::Random sampler_;

    static uint32_t constexpr Num_material_samplers = 3;
    sampler::Golden_ratio material_samplers_[Num_material_samplers];

    static uint32_t constexpr Num_light_samplers = 3;
    sampler::Golden_ratio light_samplers_[Num_light_samplers];
};

class Pathtracer_MIS_factory final : public Factory {
  public:
    Pathtracer_MIS_factory(take::Settings const& take_settings, uint32_t num_integrators,
                           uint32_t num_samples, uint32_t min_bounces, uint32_t max_bounces,
                           Light_sampling light_sampling, bool enable_caustics) noexcept;

    ~Pathtracer_MIS_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Pathtracer_MIS* integrators_;

    Pathtracer_MIS::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
