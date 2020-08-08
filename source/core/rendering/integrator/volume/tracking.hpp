#ifndef SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_HPP
#define SU_CORE_RENDERING_INTEGRATOR_VOLUME_TRACKING_HPP

#include "base/math/vector3.hpp"
#include "scene/material/sampler_settings.hpp"

namespace math {
struct ray;
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
struct CCE;
struct CM;
}  // namespace material

namespace prop {
struct Intersection;
}

struct Ray;

}  // namespace scene

namespace rendering {

class Worker;

enum class Event;

namespace integrator::volume {

class Tracking {
  public:
    using Ray            = scene::Ray;
    using Transformation = scene::entity::Composed_transformation;
    using Material       = scene::material::Material;
    using Filter         = scene::material::Sampler_settings::Filter;
    using CC             = scene::material::CC;
    using CCE            = scene::material::CCE;
    using CM             = scene::material::CM;
    using Intersection   = scene::prop::Intersection;

    static bool transmittance(Ray const& ray, Worker& worker, float3& tr);

    static bool tracking(ray const& ray, CM const& cm, Material const& material, float srs,
                         Filter filter, Worker& worker, float& t_out, float3& w);

    static Event tracking(ray const& ray, CM const& cm, Material const& material, float srs,
                          Filter filter, Worker& worker, float& t_out, float3& w, float3& li);

    static bool tracking(ray const& ray, CC const& mu, rnd::Generator& rng, float& t_out,
                         float3& w);

    static Event tracking(ray const& ray, CCE const& cce, rnd::Generator& rng, float& t_out,
                          float3& w, float3& li);

    static float constexpr Min_mt        = 1e-10f;
    static float constexpr Abort_epsilon = 7.5e-4f;
};

}  // namespace integrator::volume
}  // namespace rendering

#endif
