#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_HPP

#include "base/math/vector3.hpp"
#include "scene/material/sampler_settings.hpp"

namespace math {
struct Ray;
}

namespace rnd {
class Generator;
}

namespace scene {

namespace entity {
struct Composed_transformation;
}

namespace material {
class Material;
struct CC;
struct CM;
}  // namespace material

namespace prop {
struct Intersection;
}

struct Ray;

}  // namespace scene

namespace rendering {

class Worker;

namespace integrator::volume {

class Tracking {
  public:
    using Ray            = scene::Ray;
    using Transformation = scene::entity::Composed_transformation;
    using Material       = scene::material::Material;
    using Sampler_filter = scene::material::Sampler_settings::Filter;
    using CC             = scene::material::CC;
    using CM             = scene::material::CM;
    using Intersection   = scene::prop::Intersection;

    static bool transmittance(Ray const& ray, rnd::Generator& rng, Worker& worker,
                              float3& transmittance);

    static bool tracking(math::Ray const& ray, CM const& cm, Material const& material,
                         Sampler_filter filter, rnd::Generator& rng, Worker& worker, float& t_out,
                         float3& w);

    static bool tracking(math::Ray const& ray, CC const& mu, rnd::Generator& rng, float& t_out,
                         float3& w);

    static float constexpr Min_mt        = 1e-10f;
    static float constexpr Ray_epsilon   = 5e-4f;
    static float constexpr Abort_epsilon = 2.5e-4f;
};

}  // namespace integrator::volume
}  // namespace rendering

#endif
