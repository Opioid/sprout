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

    Integrator(rnd::Generator& rng, take::Settings const& settings) noexcept;

    ~Integrator() noexcept override;

    virtual float3 li(Ray& ray, Intersection& intersection, Worker& worker,
                      Interface_stack const& initial_stack) noexcept = 0;
};

class Factory {
  public:
    Factory(take::Settings const& settings) noexcept;

    virtual ~Factory() noexcept;

    virtual Integrator* create(uint32_t id, rnd::Generator& rng) const noexcept = 0;

    virtual uint32_t max_sample_depth() const noexcept;

  protected:
    take::Settings const& take_settings_;
};

}  // namespace integrator::surface
}  // namespace rendering

#endif
