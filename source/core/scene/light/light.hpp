#ifndef SU_SCENE_LIGHT_LIGHT_HPP
#define SU_SCENE_LIGHT_LIGHT_HPP

#include "base/math/vector.hpp"
#include "scene/material/sampler_settings.hpp"

namespace math {
struct AABB;
}

namespace image::texture::sampler {
class Sampler_2D;
}

namespace sampler {
class Sampler;
}

namespace thread {
class Pool;
}

namespace scene {

namespace entity {
struct Composed_transformation;
}
namespace prop {
class Prop;
}
namespace shape {
struct Intersection;
class Node_stack;
struct Sample_to;
}  // namespace shape

class Worker;
struct Ray;

namespace light {

struct Sample_from;

class Light {
  public:
    using Prop           = prop::Prop;
    using Transformation = entity::Composed_transformation;
    using Sampler_filter = material::Sampler_settings::Filter;
    using Intersection   = shape::Intersection;
    using Sample_to      = shape::Sample_to;

    virtual ~Light();

    virtual Transformation const& transformation_at(float           time,
                                                    Transformation& transformation) const = 0;

    virtual bool sample(f_float3 p, f_float3 n, Transformation const& transformation,
                        bool total_sphere, sampler::Sampler& sampler, uint32_t sampler_dimension,
                        Worker const& worker, Sample_to& result) const = 0;

    virtual bool sample(f_float3 p, Transformation const& transformation, sampler::Sampler& sampler,
                        uint32_t sampler_dimension, Worker const& worker,
                        Sample_to& result) const = 0;

    virtual bool sample(float time, Transformation const& transformation, sampler::Sampler& sampler,
                        uint32_t sampler_dimension, Sampler_filter filter, Worker const& worker,
                        Sample_from& result) const = 0;

    virtual float3 evaluate_radiance(Sample_to const& sample, float time, Sampler_filter filter,
                                     Worker const& worker) const = 0;

    bool sample(f_float3 p, f_float3 n, float time, bool total_sphere, sampler::Sampler& sampler,
                uint32_t sampler_dimension, Worker const& worker, Sample_to& result) const;

    bool sample(f_float3 p, float time, sampler::Sampler& sampler, uint32_t sampler_dimension,
                Worker const& worker, Sample_to& result) const;

    bool sample(float time, sampler::Sampler& sampler, uint32_t sampler_dimension,
                Sampler_filter filter, Worker const& worker, Sample_from& result) const;

    virtual float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                      Sampler_filter filter, Worker const& worker) const = 0;

    virtual float3 power(math::AABB const& scene_bb) const = 0;

    virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) = 0;

    virtual bool equals(Prop const* prop, uint32_t part) const = 0;

    static bool is_light(uint32_t id);
};

}  // namespace light
}  // namespace scene

#endif
