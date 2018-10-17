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
struct Sample_from;
}  // namespace shape

class Worker;
struct Ray;

namespace light {

class Light {
  public:
    using Prop           = prop::Prop;
    using Transformation = entity::Composed_transformation;
    using Sampler_filter = material::Sampler_settings::Filter;
    using Intersection   = shape::Intersection;
    using Sample_to      = shape::Sample_to;
    using Sample_from    = shape::Sample_from;

    virtual ~Light() noexcept;

    virtual Transformation const& transformation_at(uint64_t           time,
                                                    Transformation& transformation) const
        noexcept = 0;

    virtual bool sample(float3 const& p, float3 const& n, Transformation const& transformation,
                        bool total_sphere, sampler::Sampler& sampler, uint32_t sampler_dimension,
                        Worker const& worker, Sample_to& result) const noexcept = 0;

    virtual bool sample(float3 const& p, Transformation const& transformation,
                        sampler::Sampler& sampler, uint32_t sampler_dimension, Worker const& worker,
                        Sample_to& result) const noexcept = 0;

    virtual float3 evaluate(Sample_to const& sample, uint64_t time, Sampler_filter filter,
                            Worker const& worker) const noexcept = 0;

    virtual bool sample(Transformation const& transformation, sampler::Sampler& sampler,
                        uint32_t sampler_dimension, math::AABB const& bounds, Worker const& worker,
                        Sample_from& result) const noexcept = 0;

    virtual float3 evaluate(Sample_from const& sample, float time, Sampler_filter filter,
                            Worker const& worker) const noexcept = 0;

    bool sample(float3 const& p, float3 const& n, uint64_t time, bool total_sphere,
                sampler::Sampler& sampler, uint32_t sampler_dimension, Worker const& worker,
                Sample_to& result) const noexcept;

    bool sample(float3 const& p, uint64_t time, sampler::Sampler& sampler, uint32_t sampler_dimension,
                Worker const& worker, Sample_to& result) const noexcept;

    bool sample(uint64_t time, sampler::Sampler& sampler, uint32_t sampler_dimension,
                math::AABB const& bounds, Worker const& worker, Sample_from& result) const noexcept;

    virtual float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                      Sampler_filter filter, Worker const& worker) const noexcept = 0;

    virtual float3 power(math::AABB const& scene_bb) const noexcept = 0;

    virtual void prepare_sampling(uint32_t light_id, thread::Pool& pool) noexcept = 0;

    virtual bool equals(Prop const* prop, uint32_t part) const noexcept = 0;

    static bool is_light(uint32_t id) noexcept;
};

}  // namespace light
}  // namespace scene

#endif
