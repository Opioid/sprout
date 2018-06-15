#ifndef SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_NG_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_NG_HPP

#include "rendering/integrator/surface/surface_integrator.hpp"
#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "scene/scene_ray.hpp"

namespace scene {

namespace light {
class Light;
}
namespace material {
class Sample;
}

}  // namespace scene

namespace rendering::integrator::surface {

class alignas(64) Pathtracer_MIS final : public Integrator {
  public:
    struct Settings {
        uint32_t min_bounces;
        uint32_t max_bounces;
        float    path_continuation_probability;

        Light_sampling light_sampling;

        float num_light_samples_reciprocal;
        bool  avoid_caustics;
    };

    Pathtracer_MIS(rnd::Generator& rng, take::Settings const& take_settings,
                   Settings const& settings);

    virtual ~Pathtracer_MIS() override final;

    virtual void prepare(Scene const& scene, uint32_t num_samples_per_pixel) override final;

    virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

    virtual float3 li(Ray& ray, Intersection& intersection, Worker& worker) override final;

    virtual size_t num_bytes() const override final;

  private:
    float3 sample_lights(Ray const& ray, float ray_offset, Intersection& intersection,
                         const Material_sample& material_sample, bool do_mis, Sampler_filter filter,
                         Worker& worker);

    float3 evaluate_light(const Light& light, float light_weight, Ray const& history,
                          float ray_offset, uint32_t sampler_dimension, bool do_mis,
                          Intersection const& intersection, const Material_sample& material_sample,
                          Sampler_filter filter, Worker& worker);

    float3 evaluate_light(Ray const& ray, Intersection const& intersection,
                          Bxdf_sample sample_result, bool treat_as_singular, bool is_translucent,
                          Sampler_filter filter, Worker& worker, bool& pure_emissive);

    sampler::Sampler& material_sampler(uint32_t bounce);
    sampler::Sampler& light_sampler(uint32_t bounce);

    const Settings settings_;

    float num_lights_reciprocal_;

    sampler::Random sampler_;

    static uint32_t constexpr Num_material_samplers = 3;
    sampler::Golden_ratio material_samplers_[Num_material_samplers];

    static uint32_t constexpr Num_light_samplers = 3;
    sampler::Golden_ratio light_samplers_[Num_light_samplers];
};

class Pathtracer_MIS_factory final : public Factory {
  public:
    Pathtracer_MIS_factory(take::Settings const& take_settings, uint32_t num_integrators,
                           uint32_t min_bounces, uint32_t max_bounces,
                           float path_termination_probability, Light_sampling light_sampling,
                           bool enable_caustics);

    virtual ~Pathtracer_MIS_factory() override final;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

  private:
    Pathtracer_MIS* integrators_;

    Pathtracer_MIS::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
