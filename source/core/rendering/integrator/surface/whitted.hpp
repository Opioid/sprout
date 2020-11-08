#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_SURFACE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_SURFACE_HPP

#include "sampler/sampler_golden_ratio.hpp"
#include "sampler/sampler_random.hpp"
#include "surface_integrator.hpp"

namespace scene::material {
class Sample;
}

namespace rendering::integrator::surface {

class alignas(64) Whitted final : public Integrator {
  public:
    Whitted();

    void prepare(Scene const& scene, uint32_t max_samples_per_pixel) final;

    void start_pixel(RNG& rng, uint32_t num_samples) final;

    float4 li(Ray& ray, Intersection& isec, Worker& worker, Interface_stack const& initial_stack,
              AOV* aov) final;

  private:
    float3 shade(Ray const& ray, Intersection const& isec, Worker& worker);

    float3 estimate_direct_light(Ray const& ray, Intersection const& isec,
                                 Material_sample const& mat_sample, Worker& worker);

    sampler::Random sampler_;
};

class Whitted_pool final : public Typed_pool<Whitted> {
  public:
    Whitted_pool(uint32_t num_integrators);

    Integrator* get(uint32_t id) const final;
};

}  // namespace rendering::integrator::surface

#endif
