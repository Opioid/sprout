#ifndef SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP
#define SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP

#include "scene/material/sampler_settings.hpp"

#include <cstdint>

namespace scene {

namespace entity {
struct Composed_transformation;
}

namespace shape {
enum class Visibility;
}

namespace material {

namespace bxdf {
struct Sample;
enum class Type;
}  // namespace bxdf

class Material;
class Sample;

}  // namespace material

namespace light {
class Light;
struct Light_ref;
}  // namespace light

namespace prop {
class Prop;
struct Intersection;
}  // namespace prop

struct Ray;
class Scene;

}  // namespace scene

namespace rnd {
class Generator;
}

namespace rendering {

class Worker;

namespace integrator {

struct Light_sampling {
    enum class Strategy { Single, All };

    Strategy strategy;
    uint32_t num_samples;
};

class Integrator {
  public:
    using Ray             = scene::Ray;
    using Scene           = scene::Scene;
    using Transformation  = scene::entity::Composed_transformation;
    using Light           = scene::light::Light;
    using Light_ref       = scene::light::Light_ref;
    using Material        = scene::material::Material;
    using Material_sample = scene::material::Sample;
    using Filter          = scene::material::Sampler_settings::Filter;
    using Bxdf_sample     = scene::material::bxdf::Sample;
    using Bxdf_type       = scene::material::bxdf::Type;
    using Prop            = scene::prop::Prop;
    using Intersection    = scene::prop::Intersection;
    using Visibility      = scene::shape::Visibility;

    Integrator();

    virtual ~Integrator();

    virtual void prepare(Scene const& scene, uint32_t num_samples_per_pixel) = 0;

    virtual void start_pixel(rnd::Generator& rng) = 0;
};

}  // namespace integrator
}  // namespace rendering

#endif
