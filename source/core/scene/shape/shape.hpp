#ifndef SU_CORE_SCENE_SHAPE_SHAPE_HPP
#define SU_CORE_SCENE_SHAPE_SHAPE_HPP

#include "base/math/aabb.hpp"
#include "base/math/matrix.hpp"
#include "scene/material/sampler_settings.hpp"

namespace math {
struct Transformation;
}

namespace sampler {
class Sampler;
}

namespace scene {

namespace material {
class Material;
}

struct Ray;
class Worker;

namespace entity {
struct Composed_transformation;
}

namespace shape {

struct Intersection;
struct Normals;
struct Sample_to;
struct Sample_from;
class Node_stack;
class Morphable_shape;
enum class Visibility;

class Shape {
  public:
    using Filter         = material::Sampler_settings::Filter;
    using Material       = material::Material;
    using Materials      = Material const* const*;
    using Sampler        = sampler::Sampler;
    using Transformation = entity::Composed_transformation;

    virtual ~Shape() noexcept;

    AABB const& aabb() const noexcept;

    float3 object_to_texture_point(float3 const& p) const noexcept;
    float3 object_to_texture_vector(float3 const& v) const noexcept;

    virtual AABB transformed_aabb(float4x4 const& m, math::Transformation const& t) const noexcept;
    virtual AABB transformed_aabb(math::Transformation const& t) const noexcept;

    virtual uint32_t num_parts() const noexcept;

    virtual bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                           Intersection& intersection) const noexcept = 0;

    virtual bool intersect_fast(Ray& ray, Transformation const& transformation,
                                Node_stack& node_stack, Intersection& intersection) const
        noexcept = 0;

    virtual bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                           Normals& normals) const noexcept = 0;

    virtual bool intersect_p(Ray const& ray, Transformation const& transformation,
                             Node_stack& node_stack) const noexcept = 0;

    virtual float opacity(Ray const& ray, Transformation const& transformation, Materials materials,
                          Filter filter, Worker const& worker) const noexcept = 0;

    virtual Visibility thin_absorption(Ray const& ray, Transformation const& transformation,
                                       Materials materials, Filter filter, Worker const& worker,
                                       float3& ta) const noexcept = 0;

    virtual bool sample(uint32_t part, float3 const& p, float3 const& n,
                        Transformation const& transformation, float area, bool two_sided,
                        Sampler& sampler, uint32_t sampler_dimension, Node_stack& node_stack,
                        Sample_to& sample) const noexcept;

    virtual bool sample(uint32_t part, float3 const& p, Transformation const& transformation,
                        float area, bool two_sided, Sampler& sampler, uint32_t sampler_dimension,
                        Node_stack& node_stack, Sample_to& sample) const noexcept = 0;

    virtual bool sample_volume(uint32_t part, float3 const& p, Transformation const& transformation,
                               float volume, Sampler& sampler, uint32_t sampler_dimension,
                               Node_stack& node_stack, Sample_to& sample) const noexcept;

    virtual bool sample(uint32_t part, Transformation const& transformation, float area,
                        bool two_sided, Sampler& sampler, uint32_t sampler_dimension,
                        float2 const& importance_uv, AABB const& bounds, Node_stack& node_stack,
                        Sample_from& sample) const noexcept = 0;

    // All pdf functions implicitely assume that the passed
    // ray/intersection/transformation combination actually lead to a hit.
    virtual float pdf(Ray const& ray, Intersection const& intersection,
                      Transformation const& transformation, float area, bool two_sided,
                      bool total_sphere) const noexcept = 0;

    virtual float pdf_volume(Ray const& ray, Intersection const& intersection,
                             Transformation const& transformation, float volume) const noexcept = 0;

    // The following functions are used for textured lights
    // and should have the uv weight baked in!
    virtual bool sample(uint32_t part, float3 const& p, float2 uv,
                        Transformation const& transformation, float area, bool two_sided,
                        Sample_to& sample) const noexcept = 0;

    virtual bool sample(uint32_t part, float3 const& p, float3 const& uvw,
                        Transformation const& transformation, float volume, Sample_to& sample) const
        noexcept = 0;

    virtual bool sample(uint32_t part, float2 uv, Transformation const& transformation, float area,
                        bool two_sided, Sampler& sampler, uint32_t sampler_dimension,
                        float2 const& importance_uv, AABB const& bounds, Sample_from& sample) const
        noexcept = 0;

    virtual float pdf_uv(Ray const& ray, Intersection const& intersection,
                         Transformation const& transformation, float area, bool two_sided) const
        noexcept = 0;

    virtual float uv_weight(float2 uv) const noexcept = 0;
    // ---

    virtual float area(uint32_t part, float3 const& scale) const noexcept = 0;

    virtual float volume(uint32_t part, float3 const& scale) const noexcept = 0;

    virtual bool is_complex() const noexcept;
    virtual bool is_finite() const noexcept;
    virtual bool is_analytical() const noexcept;

    virtual void prepare_sampling(uint32_t part) noexcept;

    virtual Morphable_shape* morphable_shape() noexcept;

    virtual size_t num_bytes() const noexcept = 0;

  protected:
    AABB aabb_;

    float3 inv_extent_;
};

}  // namespace shape
}  // namespace scene

#endif
