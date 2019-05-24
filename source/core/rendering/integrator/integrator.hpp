#ifndef SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP
#define SU_RENDERING_INTEGRATOR_INTEGRATOR_HPP

#include <cstddef>
#include <cstdint>
#include "scene/material/sampler_settings.hpp"
#include "take/take_settings.hpp"

namespace rnd {
class Generator;
}

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
class Light;
}  // namespace light

namespace prop {
class Prop;
struct Intersection;
}  // namespace prop

struct Ray;
class Scene;

}  // namespace scene

namespace rendering::integrator {

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
    using Material        = scene::material::Material;
    using Material_sample = scene::material::Sample;
    using Filter          = scene::material::Sampler_settings::Filter;
    using Bxdf_sample     = scene::material::bxdf::Sample;
    using Bxdf_type       = scene::material::bxdf::Type;
    using Prop            = scene::prop::Prop;
    using Intersection    = scene::prop::Intersection;
    using Visibility      = scene::shape::Visibility;

    Integrator(rnd::Generator& rng, take::Settings const& settings) noexcept;

    virtual ~Integrator() noexcept;

    virtual void prepare(Scene const& scene, uint32_t num_samples_per_pixel) noexcept = 0;

    virtual void start_pixel() noexcept = 0;

    virtual size_t num_bytes() const noexcept = 0;

  protected:
    rnd::Generator& rng_;

    take::Settings const take_settings_;
};

}  // namespace rendering::integrator

#endif
