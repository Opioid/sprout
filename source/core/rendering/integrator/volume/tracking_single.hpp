#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP

#include "sampler/sampler_random.hpp"
#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Tracking_single final : public Integrator {
  public:
    Tracking_single(rnd::Generator& rng, take::Settings const& take_settings);

    virtual void prepare(Scene const& scene, uint32_t num_samples_per_pixel) override final;

    virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

    virtual float3 transmittance(Ray const& ray, Worker& worker) override final;

    virtual bool integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
                           Worker& worker, float3& li, float3& transmittance) override final;

    virtual size_t num_bytes() const override final;

  private:
    float3 direct_light(Ray const& ray, f_float3 position, Worker& worker);

    float3 direct_light(Ray const& ray, f_float3 position, Intersection const& intersection,
                        Worker& worker);

    sampler::Random sampler_;
};

class Tracking_single_factory final : public Factory {
  public:
    Tracking_single_factory(take::Settings const& take_settings, uint32_t num_integrators);

    ~Tracking_single_factory() override final;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

  private:
    Tracking_single* integrators_;
};

}  // namespace rendering::integrator::volume

#endif
