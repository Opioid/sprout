#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_SINGLE_HPP

#include "sampler/sampler_random.hpp"
#include "volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Tracking_single final : public Integrator {
  public:
    Tracking_single(bool progressive);

    ~Tracking_single();

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) final;

    void start_pixel(rnd::Generator& rng) final;

    bool transmittance(Ray const& ray, Worker& worker, float3& tr) final;

    Event integrate(Ray& ray, Intersection& intersection, Filter filter, Worker& worker, float3& li,
                    float3& tr) final;

  private:
    float3 direct_light(Light const& light, float light_pdf, Ray const& ray, float3 const& position,
                        Intersection const& intersection, Worker& worker);

    float3 one_bounce(Ray const& ray, Intersection const& intersection, Material const& material,
                      Worker& worker);

    sampler::Sampler& material_sampler(uint32_t bounce);

    sampler::Sampler& light_sampler(uint32_t bounce);

    sampler::Random sampler_;

    sampler::Pool* sampler_pool_;

    static uint32_t constexpr Num_dedicated_samplers = 1;

    sampler::Sampler* material_samplers_[Num_dedicated_samplers];

    sampler::Sampler* light_samplers_[Num_dedicated_samplers];
};

class Tracking_single_pool final : public Typed_pool<Tracking_single> {
  public:
    Tracking_single_pool(uint32_t num_integrators, bool progressive);

    Integrator* get(uint32_t id) const final;

  private:
    bool progressive_;
};

}  // namespace rendering::integrator::volume

#endif
