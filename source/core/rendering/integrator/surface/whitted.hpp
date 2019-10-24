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
    struct Settings {
        uint32_t num_light_samples;
        float    num_light_samples_reciprocal;
    };

    Whitted(rnd::Generator& rng, take::Settings const& take_settings,
            Settings const& settings) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    float4 li(Ray& ray, Intersection& intersection, Worker& worker,
              Interface_stack const& initial_stack) noexcept override final;

  private:
    float3 shade(Ray const& ray, Intersection const& intersection, Worker& worker) noexcept;

    float3 estimate_direct_light(Ray const& ray, Intersection const& intersection,
                                 Material_sample const& material_sample, Worker& worker) noexcept;

    Settings const settings_;

    sampler::Random sampler_;
};

class Whitted_factory final : public Factory {
  public:
    Whitted_factory(take::Settings const& take_settings, uint32_t num_integrators,
                    uint32_t num_light_samples) noexcept;

    ~Whitted_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Whitted* integrators_;

    Whitted::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
