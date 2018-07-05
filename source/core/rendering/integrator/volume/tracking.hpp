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
}

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
    using Intersection   = scene::prop::Intersection;

    static float3 transmittance(Ray const& ray, rnd::Generator& rng, Worker& worker);

    static bool track(math::Ray const& ray, float mt, Material const& material,
                      Sampler_filter filter, rnd::Generator& rng, Worker& worker, float& t,
                      float3& w);

    // Completely arbitrary and biased cutoff limit in order to prevent some worst case things
    static uint32_t constexpr max_iterations_ = 1024 * 128;

    static float constexpr Min_mt      = 1e-10f;
    static float constexpr Ray_epsilon = 3e-4f;

  private:
    static float3 track_transmittance(math::Ray const& ray, float mt, Material const& material,
                                      Sampler_filter filter, rnd::Generator& rng, Worker& worker);
};

}  // namespace integrator::volume
}  // namespace rendering

#endif
