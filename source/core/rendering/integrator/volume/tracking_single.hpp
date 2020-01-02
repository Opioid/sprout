#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP

#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Tracking_single final : public Integrator {
  public:
    Tracking_single(rnd::Generator& rng) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    bool transmittance(Ray const& ray, Worker& worker, float3& tr) noexcept override final;

    Event integrate(Ray& ray, Intersection& intersection, Filter filter, Worker& worker, float3& li,
                    float3& tr) noexcept override final;

  private:
    float3 direct_light(Ray const& ray, float3 const& position, Intersection const& intersection,
                        Worker& worker) noexcept;

    float3 direct_light(Light const& light, float light_pdf, Ray const& ray, float3 const& position,
                        Intersection const& intersection, Worker& worker) noexcept;

    sampler::Sampler& material_sampler(uint32_t bounce) noexcept;

    sampler::Sampler& light_sampler(uint32_t bounce) noexcept;

    sampler::Random sampler_;

    static uint32_t constexpr Num_material_samplers = 1;
    sampler::Golden_ratio material_samplers_[Num_material_samplers];

    static uint32_t constexpr Num_light_samplers = 1;
    sampler::Golden_ratio light_samplers_[Num_light_samplers];
};

class Tracking_single_pool final : public Typed_pool<Tracking_single> {
  public:
    Tracking_single_pool(uint32_t num_integrators) noexcept;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;
};

}  // namespace rendering::integrator::volume

#endif
