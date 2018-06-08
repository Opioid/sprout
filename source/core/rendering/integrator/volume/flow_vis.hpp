#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_FLOW_VIS_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_FLOW_VIS_HPP

#include "rendering/integrator/volume/volume_integrator.hpp"

namespace rendering::integrator::volume {

class alignas(64) Flow_vis final : public Integrator {
  public:
    struct Settings {
        float step_size;
    };

    Flow_vis(rnd::Generator& rng, take::Settings const& take_settings, Settings const& settings);

    virtual void prepare(scene::Scene const& scene, uint32_t num_samples_per_pixel) override final;

    virtual void resume_pixel(uint32_t sample, rnd::Generator& scramble) override final;

    virtual float3 transmittance(Ray const& ray, Worker& worker) override final;

    virtual bool integrate(Ray& ray, Intersection& intersection, Sampler_filter filter,
                           Worker& worker, float3& li, float3& transmittance) override final;

    virtual size_t num_bytes() const override final;

    const Settings settings_;
};

class Flow_vis_factory : public Factory {
  public:
    Flow_vis_factory(take::Settings const& settings, uint32_t num_integrators, float step_size);

    virtual ~Flow_vis_factory() override;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const override final;

  private:
    Flow_vis* integrators_;

    const Flow_vis::Settings settings_;
};

}  // namespace rendering::integrator::volume

#endif
