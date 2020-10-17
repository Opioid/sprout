#ifndef SU_SCENE_LIGHT_LIGHT_HPP
#define SU_SCENE_LIGHT_LIGHT_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "base/math/vector.hpp"
#include "scene/material/sampler_settings.hpp"

namespace math {
struct AABB;
}

namespace image::texture {
class Sampler_2D;
}

namespace sampler {
class Sampler;
}

namespace thread {
class Pool;
}

using Threads = thread::Pool;

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

static uint32_t constexpr Null = 0xFFFFFFFF;

class alignas(16) Light {
  public:
    using Transformation = entity::Composed_transformation;
    using Filter         = material::Sampler_settings::Filter;
    using Intersection   = shape::Intersection;
    using Sample_to      = shape::Sample_to;
    using Sample_from    = shape::Sample_from;
    using Sampler        = sampler::Sampler;

    enum class Type { Null, Prop, Prop_image, Volume, Volume_image };

    Light();

    Light(Type type, uint32_t prop, uint32_t part);

    float area() const;

    void set_extent(float extent);

    Transformation const& transformation_at(uint64_t time, Transformation& trafo,
                                            Scene const& scene) const;

    bool is_finite(Scene const& scene) const;

    bool sample(float3 const& p, float3 const& n, Transformation const& trafo, bool total_sphere,
                Sampler& sampler, uint32_t sampler_d, Worker& worker, Sample_to& result) const;

    float3 evaluate(Sample_to const& sample, Filter filter, Worker const& worker) const;

    bool sample(Transformation const& trafo, Sampler& sampler, uint32_t sampler_d,
                AABB const& bounds, Worker& worker, Sample_from& result) const;

    bool sample(Transformation const& trafo, Sampler& sampler, uint32_t sampler_d,
                Distribution_2D const& importance, AABB const& bounds, Worker& worker,
                Sample_from& result) const;

    float3 evaluate(Sample_from const& sample, Filter filter, Worker const& worker) const;

    bool sample(float3 const& p, float3 const& n, uint64_t time, bool total_sphere,
                Sampler& sampler, uint32_t sampler_d, Worker& worker, Sample_to& result) const;

    bool sample(float3 const& p, uint64_t time, Sampler& sampler, uint32_t sampler_d,
                Worker& worker, Sample_to& result) const;

    bool sample(uint64_t time, Sampler& sampler, uint32_t sampler_d, AABB const& bounds,
                Worker& worker, Sample_from& result) const;

    bool sample(uint64_t time, Sampler& sampler, uint32_t sampler_d,
                Distribution_2D const& importance, AABB const& bounds, Worker& worker,
                Sample_from& result) const;

    float pdf(Ray const& ray, Intersection const& isec, bool total_sphere, Filter filter,
              Worker const& worker) const;

    float3 power(AABB const& scene_bb, Scene const& scene) const;

    void prepare_sampling(uint32_t light_id, uint64_t time, Scene& scene, Threads& threads) const;

    bool equals(uint32_t prop, uint32_t part) const;

    static uint32_t constexpr Volume_light_mask = 0x40000000;

    static bool is_light(uint32_t id);

    static bool is_area_light(uint32_t id);

    static uint32_t strip_mask(uint32_t id);

  private:
    Type type_;

    uint32_t prop_;
    uint32_t part_;

    float extent_;
};

struct Light_pick {
    uint32_t id;

    float pdf;
};

}  // namespace light
}  // namespace scene

#endif
