#ifndef SU_RENDERING_INTEGRATOR_SURFACE_LIGHTTRACER_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_LIGHTTRACER_HPP

#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "scene/scene_ray.hpp"
#include "surface_integrator.hpp"

namespace scene::light {
struct Sample;
}

namespace rendering::integrator::surface {

class alignas(64) Lighttracer final : public Integrator {
  public:
    struct Settings {
        uint32_t min_bounces;
        uint32_t max_bounces;
        float    path_continuation_probability;
    };

    Lighttracer(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings);

    virtual ~Lighttracer() override final;

    virtual void prepare(Scene const& scene, uint32_t num_samples_per_pixel) override final;

    virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

    virtual float3 li(Ray& ray, Intersection& intersection, Worker& worker) override final;

    virtual size_t num_bytes() const override final;

  private:
    Ray generate_light_ray();

    sampler::Sampler& material_sampler(uint32_t bounce);

    const Settings settings_;

    sampler::Random sampler_;

    static uint32_t constexpr Num_material_samplers = 3;
    sampler::Golden_ratio material_samplers_[Num_material_samplers];
};

class Lighttracer_factory final : public Factory {
  public:
    Lighttracer_factory(take::Settings const& take_settings, uint32_t num_integrators,
                        uint32_t min_bounces, uint32_t max_bounces,
                        float path_termination_probability);

    virtual ~Lighttracer_factory() override final;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

  private:
    Lighttracer* integrators_;

    Lighttracer::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
