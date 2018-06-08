#ifndef SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DL1
#define SU_CORE_RENDERING_INTEGRATOR_SURFACE_PATHTRACER_DL1

#include "sampler/sampler_random.hpp"
#include "scene/material/sampler_settings.hpp"
#include "surface_integrator.hpp"

namespace scene::material {
class Sample;
}

namespace rendering::integrator::surface {

class alignas(64) Pathtracer_DL final : public Integrator {
 public:
  struct Settings {
    uint32_t min_bounces;
    uint32_t max_bounces;
    float path_continuation_probability;

    uint32_t num_light_samples;
    float num_light_samples_reciprocal;
    bool disable_caustics;
  };

  Pathtracer_DL(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings);

  virtual void prepare(Scene const& scene, uint32_t num_samples_per_pixel) override final;

  virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

  virtual float3 li(Ray& ray, Intersection& intersection, Worker& worker) override final;

  virtual size_t num_bytes() const override final;

 private:
  float3 direct_light(Ray const& ray, Intersection const& intersection,
                      const Material_sample& material_sample, Sampler_filter filter,
                      Worker& worker);

  const Settings settings_;

  sampler::Random sampler_;
};

class Pathtracer_DL_factory final : public Factory {
 public:
  Pathtracer_DL_factory(take::Settings const& take_settings, uint32_t num_integrators,
                        uint32_t min_bounces, uint32_t max_bounces,
                        float path_termination_probability, uint32_t num_light_samples,
                        bool enable_caustics);

  virtual ~Pathtracer_DL_factory() override final;

  virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

 private:
  Pathtracer_DL* integrators_;

  Pathtracer_DL::Settings settings_;
};

}  // namespace rendering::integrator::surface

#endif
