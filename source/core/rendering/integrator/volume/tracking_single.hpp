#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP

#include "sampler/sampler_random.hpp"
#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Tracking_single final : public Integrator {
  public:
    Tracking_single(rnd::Generator& rng, take::Settings const& take_settings) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void resume_pixel(uint32_t sample, rnd::Generator& scramble) noexcept override final;

    bool transmittance(Ray const& ray, Worker& worker,
                               float3& transmittance) noexcept override final;

    bool integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
                           Worker& worker, float3& li, float3& transmittance) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    float3 direct_light(Ray const& ray, f_float3 position, Intersection const& intersection,
                        Worker& worker) noexcept;

    sampler::Random sampler_;
};

class Tracking_single_factory final : public Factory {
  public:
    Tracking_single_factory(take::Settings const& take_settings, uint32_t num_integrators) noexcept;

    ~Tracking_single_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Tracking_single* integrators_;
};

}  // namespace rendering::integrator::volume

#endif
