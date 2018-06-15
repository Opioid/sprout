#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_AERIAL_PERSPECTIVE_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_AERIAL_PERSPECTIVE_HPP

#include "sampler/sampler_random.hpp"
#include "volume_integrator.hpp"

namespace scene::entity {
struct Composed_transformation;
}

namespace rendering::integrator::volume {

class alignas(64) Aerial_perspective final : public Integrator {
  public:
    struct Settings {
        float step_size;

        bool disable_shadows;
    };

    Aerial_perspective(rnd::Generator& rng, take::Settings const& take_settings,
                       Settings const& settings);

    virtual void prepare(Scene const& scene, uint32_t num_samples_per_pixel) override final;

    virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

    virtual float3 transmittance(Ray const& ray, Worker& worker) override final;

    virtual bool integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
                           Worker& worker, float3& li, float3& transmittance) override final;

    virtual size_t num_bytes() const override final;

  private:
    /*
    float3 integrate_with_shadows(Ray const& ray, const Volume& volume,
                                                              Worker& worker, float3&
    transmittance);

    float3 integrate_without_shadows(Ray const& ray, const Volume& volume,
                                                                     Worker& worker, float3&
    transmittance);
    */

    const Material_sample& sample(f_float3 wo, float time, const Material& material,
                                  Sampler_filter filter, Worker& worker, uint32_t depth);

    const Settings settings_;

    sampler::Random sampler_;
};

class Aerial_perspective_factory : public Factory {
  public:
    Aerial_perspective_factory(take::Settings const& take_settings, uint32_t num_integrators,
                               float step_size, bool shadows);

    virtual ~Aerial_perspective_factory() override;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

  private:
    Aerial_perspective* integrators_;

    Aerial_perspective::Settings settings_;
};

}  // namespace rendering::integrator::volume

#endif
