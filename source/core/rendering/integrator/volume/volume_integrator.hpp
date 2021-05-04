#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_INTEGRATOR_HPP

#include "base/math/vector4.hpp"
#include "rendering/integrator/integrator.hpp"

namespace scene::shape {
struct Normals;
}

namespace rendering {

class Worker;
enum class Event;

namespace integrator::volume {

class Integrator : public integrator::Integrator {
  public:
    using Normals = scene::shape::Normals;

    Integrator();

    virtual ~Integrator();

    virtual bool transmittance(Ray const& ray, Filter filter, Worker& worker, float3& tr) = 0;

    virtual Event integrate(Ray& ray, Intersection& isec, Filter filter, Worker& worker, float3& li,
                            float3& tr) = 0;
};

using Pool = integrator::Pool<Integrator>;

template <typename T>
using Typed_pool = Typed_pool<T, Integrator>;

}  // namespace integrator
}  // namespace rendering

#endif
