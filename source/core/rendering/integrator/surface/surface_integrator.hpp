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

class Pool {
  public:
    Pool(uint32_t num_integrators) noexcept;

    virtual ~Pool() noexcept;

    virtual Integrator* get(uint32_t id, rnd::Generator& rng) const noexcept = 0;

  protected:
    uint32_t num_integrators_;
};

template <typename T>
class Typed_pool : public Pool {
  public:
    Typed_pool(uint32_t num_integrators) noexcept;

    ~Typed_pool() noexcept override;

  protected:
    T* integrators_;
};

}  // namespace integrator::surface
}  // namespace rendering

#endif
