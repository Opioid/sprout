#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP

#include "sampler/sampler_random.hpp"
#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Tracking_single final : public Integrator {
  public:
    Tracking_single(uint32_t max_samples_per_pixel, bool progressive);

    ~Tracking_single();

    void start_pixel(RNG& rng) final;

    bool transmittance(Ray const& ray, Worker& worker, float3& tr) final;

    Event integrate(Ray& ray, Intersection& isec, Filter filter, Worker& worker, float3& li,
                    float3& tr) final;

  private:
    float3 direct_light(Light const& light, float light_weight, Ray const& ray, float3_p position,
                        uint32_t sampler_d, Intersection const& isec, Material const& material,
                        Worker& worker);

    float3 one_bounce(Ray const& ray, Material const& material, Worker& worker);

    sampler::Sampler& material_sampler(uint32_t bounce);

    sampler::Sampler& light_sampler(uint32_t bounce);

    sampler::Random sampler_;

    sampler::Pool* sampler_pool_;

    static uint32_t constexpr Num_dedicated_samplers = 1;
};

class Tracking_single_pool final : public Typed_pool<Tracking_single> {
  public:
    Tracking_single_pool(uint32_t num_integrators, bool progressive);

    Integrator* create(uint32_t id, uint32_t max_samples_per_pixel) const final;

  private:
    bool progressive_;
};

}  // namespace rendering::integrator::volume

#endif
