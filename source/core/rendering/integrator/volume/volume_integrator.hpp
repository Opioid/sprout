#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP

#include "base/math/vector4.hpp"
#include "rendering/integrator/integrator.hpp"

namespace rendering {

class Worker;
enum class Event;

namespace integrator::volume {

class Integrator : public integrator::Integrator {
  public:
    Integrator(rnd::Generator& rng) noexcept;

    virtual ~Integrator() noexcept;

    virtual bool transmittance(Ray const& ray, Worker& worker, float3& transmittance) noexcept = 0;

    virtual Event integrate(Ray& ray, Intersection& intersection, Filter filter, Worker& worker,
                            float3& li, float3& transmittance) noexcept = 0;
};

class Factory {
  public:
    Factory() noexcept;

    virtual ~Factory() noexcept;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept = 0;
};

}  // namespace integrator::volume
}  // namespace rendering

#endif
