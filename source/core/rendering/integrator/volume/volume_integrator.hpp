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

class Pool {
  public:
    Pool(uint32_t num_integrators);

    virtual ~Pool();

    virtual Integrator* create(uint32_t id, uint32_t max_samples_per_pixel) const = 0;

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

}  // namespace integrator::volume
}  // namespace rendering

#endif
