#ifndef SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP
#define SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP

#include "base/memory/array.hpp"
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
struct Light_pick;
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

using RNG = rnd::Generator;

namespace rendering {

class Worker;

namespace integrator {

enum class Light_sampling { Single, Adaptive, All };

class Integrator {
  public:
    using Ray             = scene::Ray;
    using Scene           = scene::Scene;
    using Transformation  = scene::entity::Composed_transformation;
    using Light           = scene::light::Light;
    using Lights          = memory::Array<scene::light::Light_pick>;
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

    virtual void prepare(Scene const& scene, uint32_t max_samples_per_pixel) = 0;

    virtual void start_pixel(RNG& rng, uint32_t num_samples) = 0;
};

}  // namespace integrator
}  // namespace rendering

#endif
