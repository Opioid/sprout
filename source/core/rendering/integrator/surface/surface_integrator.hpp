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

namespace integrator {

namespace surface {

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

using Pool = integrator::Pool<Integrator>;

template<typename T>
using Typed_pool = Typed_pool<T, Integrator>;

}  // namespace surface

extern template class Pool<surface::Integrator>;

}  // namespace integrator
}  // namespace rendering

#endif
