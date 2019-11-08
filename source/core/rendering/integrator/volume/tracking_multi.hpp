#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_MULTI_HPP

#include "volume_integrator.hpp"

namespace scene::prop {
struct Interface;
}

namespace rendering::integrator::volume {

class alignas(64) Tracking_multi final : public Integrator {
  public:
    Tracking_multi(rnd::Generator& rng) noexcept;

    void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept override final;

    void start_pixel() noexcept override final;

    bool transmittance(Ray const& ray, Worker& worker,
                       float3& transmittance) noexcept override final;

    Event integrate(Ray& ray, Intersection& intersection, Filter filter, Worker& worker, float3& li,
                    float3& transmittance) noexcept override final;

  private:
    using Interface = scene::prop::Interface;

    static void set_scattering(Intersection& intersection, Interface const* interface,
                               float3 const& p) noexcept;
};

class Tracking_multi_factory final : public Factory {
  public:
    Tracking_multi_factory(uint32_t num_integrators) noexcept;

    ~Tracking_multi_factory() noexcept override final;

    Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept override final;

  private:
    Tracking_multi* integrators_;
};

}  // namespace rendering::integrator::volume

#endif
