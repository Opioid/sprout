#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP

#include "base/math/vector4.hpp"
#include "rendering/integrator/integrator.hpp"

namespace rendering {

class Worker;

namespace integrator::volume {

class Integrator : public integrator::Integrator {
  public:
    Integrator(rnd::Generator& rng, take::Settings const& settings) noexcept;

    virtual ~Integrator() noexcept;

    virtual bool transmittance(Ray const& ray, Worker& worker, float3& transmittance) noexcept = 0;

    virtual bool integrate(Ray& ray, Intersection& intersection, Filter filter, Worker& worker,
                           float3& li, float3& transmittance) noexcept = 0;
};

class Factory {
  public:
    Factory(take::Settings const& settings, uint32_t num_integrators) noexcept;

    virtual ~Factory() noexcept;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept = 0;

  protected:
    take::Settings const& take_settings_;

    uint32_t const num_integrators_;
};

}  // namespace integrator::volume
}  // namespace rendering

#endif
