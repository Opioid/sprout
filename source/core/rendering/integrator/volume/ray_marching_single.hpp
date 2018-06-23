#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_RAY_MARCHING_SINGLE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_RAY_MARCHING_SINGLE_HPP

#include "sampler/sampler_random.hpp"
#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Ray_marching_single : public Integrator {
  public:
    struct Settings {
        float step_size;
        float step_probability;
    };

    Ray_marching_single(rnd::Generator& rng, take::Settings const& take_settings,
                        Settings const& settings);

    virtual void prepare(Scene const& scene, uint32_t num_samples_per_pixel) override final;

    virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

    virtual float3 transmittance(Ray const& ray, Worker& worker) override final;

    virtual bool integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
                           Worker& worker, float3& li, float3& transmittance) override final;

    virtual size_t num_bytes() const override final;

  private:
    float3 estimate_direct_light(Ray const& ray, f_float3 position, Worker& worker);

    const Settings settings_;

    sampler::Random sampler_;
};

class Ray_marching_single_factory : public Factory {
  public:
    Ray_marching_single_factory(take::Settings const& take_settings, uint32_t num_integrators,
                                float step_size, float step_probability);

    virtual ~Ray_marching_single_factory() override;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

  private:
    Ray_marching_single* integrators_;

    Ray_marching_single::Settings settings_;
};

}  // namespace rendering::integrator::volume

#endif
