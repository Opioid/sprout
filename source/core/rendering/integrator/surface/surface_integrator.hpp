#ifndef SU_RENDERING_INTEGRATOR_SURFACE_INTEGRATOR_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_INTEGRATOR_HPP

#include "base/math/vector.hpp"
#include "rendering/integrator/integrator.hpp"

namespace rendering {

class Worker;

namespace integrator::surface {

class Integrator : public integrator::Integrator {
  public:
    Integrator(rnd::Generator& rng, take::Settings const& settings);
    virtual ~Integrator();

    virtual float3 li(Ray& ray, Intersection& intersection, Worker& worker) = 0;
};

class Factory {
  public:
    Factory(take::Settings const& settings);
    virtual ~Factory();

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const = 0;

    virtual uint32_t max_sample_depth() const;

  protected:
    take::Settings const& take_settings_;
};

}  // namespace integrator::surface
}  // namespace rendering

#endif
