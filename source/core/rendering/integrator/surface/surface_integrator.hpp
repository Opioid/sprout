#ifndef SU_RENDERING_INTEGRATOR_SURFACE_INTEGRATOR_HPP
#define SU_RENDERING_INTEGRATOR_SURFACE_INTEGRATOR_HPP

#include "base/math/vector.hpp"
#include "rendering/integrator/integrator.hpp"

namespace scene::prop {
class Interface_stack;
}

namespace rendering {

namespace sensor::aov {
class Value;
}

namespace integrator::surface {

class Integrator : public integrator::Integrator {
  public:
    using Interface_stack = scene::prop::Interface_stack;
    using AOV             = sensor::aov::Value;

    Integrator();

    ~Integrator() override;

    virtual float4 li(Ray& ray, Intersection& isec, Worker& worker,
                      Interface_stack const& initial_stack, AOV* aov) = 0;

  protected:
    void common_AOVs(float3_p throughput, Ray const& ray, Intersection const& isec,
                     Material_sample const& mat_sample, bool primary_ray, Worker& worker, AOV& aov);
};

class Pool {
  public:
    Pool(uint32_t num_integrators);

    virtual ~Pool();

    virtual Integrator* get(uint32_t id) const = 0;

  protected:
    uint32_t num_integrators_;
};

template <typename T>
class Typed_pool : public Pool {
  public:
    Typed_pool(uint32_t num_integrators);

    ~Typed_pool() override;

  protected:
    T* integrators_;
};

}  // namespace integrator::surface
}  // namespace rendering

#endif
