#ifndef SU_SCENE_LIGHT_LIGHT_HPP
#define SU_SCENE_LIGHT_LIGHT_HPP

#include "base/math/distribution/distribution_2d.hpp"
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

class Scene;
class Worker;
struct Ray;

namespace light {

class Light {
  public:
    using AABB           = math::AABB;
    using Prop           = prop::Prop;
    using Transformation = entity::Composed_transformation;
    using Filter         = material::Sampler_settings::Filter;
    using Intersection   = shape::Intersection;
    using Sample_to      = shape::Sample_to;
    using Sample_from    = shape::Sample_from;
    using Sampler        = sampler::Sampler;

    virtual ~Light() noexcept;

    virtual Transformation const& transformation_at(uint64_t time, Transformation& transformation,
                                                    Scene const& scene) const noexcept = 0;

    virtual bool sample(float3 const& p, float3 const& n, Transformation const& transformation,
                        bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                        Worker const& worker, Sample_to& result) const noexcept = 0;

    virtual float3 evaluate(Sample_to const& sample, Filter filter, Worker const& worker) const
        noexcept = 0;

    virtual bool sample(Transformation const& transformation, Sampler& sampler,
                        uint32_t sampler_dimension, AABB const& bounds, Worker const& worker,
                        Sample_from& result) const noexcept = 0;

    virtual bool sample(Transformation const& transformation, Sampler& sampler,
                        uint32_t sampler_dimension, Distribution_2D const& importance,
                        AABB const& bounds, Worker const& worker, Sample_from& result) const
        noexcept = 0;

    virtual float3 evaluate(Sample_from const& sample, Filter filter, Worker const& worker) const
        noexcept = 0;

    bool sample(float3 const& p, float3 const& n, uint64_t time, bool total_sphere,
                Sampler& sampler, uint32_t sampler_dimension, Worker const& worker,
                Sample_to& result) const noexcept;

    bool sample(float3 const& p, uint64_t time, Sampler& sampler, uint32_t sampler_dimension,
                Worker const& worker, Sample_to& result) const noexcept;

    bool sample(uint64_t time, Sampler& sampler, uint32_t sampler_dimension, AABB const& bounds,
                Worker const& worker, Sample_from& result) const noexcept;

    bool sample(uint64_t time, Sampler& sampler, uint32_t sampler_dimension,
                Distribution_2D const& importance, AABB const& bounds, Worker const& worker,
                Sample_from& result) const noexcept;

    virtual float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere,
                      Filter filter, Worker const& worker) const noexcept = 0;

    virtual float3 power(AABB const& scene_bb, Scene const& scene) const noexcept = 0;

    virtual void prepare_sampling(uint32_t light_id, uint64_t time, Scene& scene,
                                  thread::Pool& pool) noexcept = 0;

    virtual bool equals(uint32_t prop, uint32_t part) const noexcept = 0;

    static uint32_t constexpr Volume_light_mask = 0x40000000;

    static bool is_light(uint32_t id) noexcept;

    static bool is_area_light(uint32_t id) noexcept;

    static uint32_t strip_mask(uint32_t id) noexcept;
};

class alignas(16) NewLight {
  public:
    using Transformation = entity::Composed_transformation;
    using Filter         = material::Sampler_settings::Filter;
    using Intersection   = shape::Intersection;
    using Sample_to      = shape::Sample_to;
    using Sample_from    = shape::Sample_from;
    using Sampler        = sampler::Sampler;

    enum class Type { Null, Prop, Prop_image, Volume, Volume_image };

    NewLight();

    NewLight(Type type, uint32_t prop, uint32_t part);

    Transformation const& transformation_at(uint64_t time, Transformation& transformation,
                                            Scene const& scene) const noexcept;

    bool sample(float3 const& p, float3 const& n, Transformation const& transformation,
                bool total_sphere, Sampler& sampler, uint32_t sampler_dimension,
                Worker const& worker, Sample_to& result) const noexcept;

    float3 evaluate(Sample_to const& sample, Filter filter, Worker const& worker) const noexcept;

    bool sample(Transformation const& transformation, Sampler& sampler, uint32_t sampler_dimension,
                AABB const& bounds, Worker const& worker, Sample_from& result) const noexcept;

    bool sample(Transformation const& transformation, Sampler& sampler, uint32_t sampler_dimension,
                Distribution_2D const& importance, AABB const& bounds, Worker const& worker,
                Sample_from& result) const noexcept;

    float3 evaluate(Sample_from const& sample, Filter filter, Worker const& worker) const noexcept;

    bool sample(float3 const& p, float3 const& n, uint64_t time, bool total_sphere,
                Sampler& sampler, uint32_t sampler_dimension, Worker const& worker,
                Sample_to& result) const noexcept;

    bool sample(float3 const& p, uint64_t time, Sampler& sampler, uint32_t sampler_dimension,
                Worker const& worker, Sample_to& result) const noexcept;

    bool sample(uint64_t time, Sampler& sampler, uint32_t sampler_dimension, AABB const& bounds,
                Worker const& worker, Sample_from& result) const noexcept;

    bool sample(uint64_t time, Sampler& sampler, uint32_t sampler_dimension,
                Distribution_2D const& importance, AABB const& bounds, Worker const& worker,
                Sample_from& result) const noexcept;

    float pdf(Ray const& ray, Intersection const& intersection, bool total_sphere, Filter filter,
              Worker const& worker) const noexcept;

    float3 power(AABB const& scene_bb, Scene const& scene) const noexcept;

    void prepare_sampling(uint32_t light_id, uint64_t time, Scene& scene, thread::Pool& pool) const
        noexcept;

    bool equals(uint32_t prop, uint32_t part) const noexcept;

    static uint32_t constexpr Volume_light_mask = 0x40000000;

    static bool is_light(uint32_t id) noexcept;

    static bool is_area_light(uint32_t id) noexcept;

    static uint32_t strip_mask(uint32_t id) noexcept;

  private:
    Type type_;

    uint32_t prop_;
    uint32_t part_;
};

}  // namespace light
}  // namespace scene

#endif
