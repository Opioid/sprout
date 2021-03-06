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

enum class Light_sampling : uint8_t { Single, Adaptive };

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

    Integrator();

    virtual ~Integrator();

    virtual void start_pixel(RNG& rng) = 0;
};

template <typename B>
class Pool {
  public:
    virtual ~Pool();

    virtual B* create(uint32_t id, uint32_t max_samples_per_pixel) const = 0;

  protected:
};

template <typename T, typename B>
class Typed_pool : public Pool<B> {
  public:
    Typed_pool(uint32_t num_integrators);

    ~Typed_pool() override;

  protected:
    uint32_t num_integrators_;

    T* integrators_;
};

}  // namespace integrator
}  // namespace rendering

#endif
