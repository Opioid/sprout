#ifndef SU_RENDERING_INTEGRATOR_SURFACE_INTEGRATOR_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_INTEGRATOR_HPP

#include "base/math/vector.hpp"
#include "rendering/integrator/integrator.hpp"

namespace scene::prop {
class Interface_stack;
}

namespace rendering {

class Worker;

namespace integrator::surface {

class Integrator : public integrator::Integrator {
  public:
    using Interface_stack = scene::prop::Interface_stack;

    Integrator(rnd::Generator& rng) noexcept;

    ~Integrator() noexcept override;

    virtual float4 li(Ray& ray, Intersection& intersection, Worker& worker,
                      Interface_stack const& initial_stack) noexcept = 0;
};

class Factory {
  public:
    Factory() noexcept;

    virtual ~Factory() noexcept;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept = 0;
};

}  // namespace integrator::surface
}  // namespace rendering

#endif
