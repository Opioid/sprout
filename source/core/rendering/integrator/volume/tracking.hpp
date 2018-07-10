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

namespace volumetric {
struct Interval_data;
}

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
    using Interval_data  = scene::material::volumetric::Interval_data;
    using Intersection   = scene::prop::Intersection;

    static float3 transmittance(Ray const& ray, rnd::Generator& rng, Worker& worker);

    static bool track(math::Ray const& ray, Interval_data const& data, Material const& material,
                      Sampler_filter filter, rnd::Generator& rng, Worker& worker, float& t,
                      float3& w);

    static float constexpr Min_mt        = 1e-10f;
    static float constexpr Ray_epsilon   = 5e-4f;
    static float constexpr Abort_epsilon = 1e-4f;
};

}  // namespace integrator::volume
}  // namespace rendering

#endif
