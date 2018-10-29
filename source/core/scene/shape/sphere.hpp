#ifndef SU_CORE_SCENE_SHAPE_SPHERE_HPP
#define SU_CORE_SCENE_SHAPE_SPHERE_HPP

#include "shape.hpp"

namespace scene::shape {

class Sphere final : public Shape {
  public:
    Sphere() noexcept;

    math::AABB transformed_aabb(float4x4 const& m, math::Transformation const& t) const
        noexcept override final;

    math::AABB transformed_aabb(math::Transformation const& t) const noexcept override final;

    bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                   Intersection& intersection) const noexcept override final;

    bool intersect_fast(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                        Intersection& intersection) const noexcept override final;

    bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                   float& epsilon) const noexcept override final;

    bool intersect_p(Ray const& ray, Transformation const& transformation,
                     Node_stack& node_stack) const noexcept override final;

    float opacity(Ray const& ray, Transformation const& transformation, Materials const& materials,
                  Filter filter, Worker const& worker) const noexcept override final;

    float3 thin_absorption(Ray const& ray, Transformation const& transformation,
                           Materials const& materials, Filter filter, Worker const& worker) const
        noexcept override final;

    bool sample(uint32_t part, float3 const& p, float3 const& n,
                Transformation const& transformation, float area, bool two_sided,
                sampler::Sampler& sampler, uint32_t sampler_dimension, Node_stack& node_stack,
                Sample_to& sample) const noexcept override final;

    bool sample(uint32_t part, float3 const& p, Transformation const& transformation, float area,
                bool two_sided, sampler::Sampler& sampler, uint32_t sampler_dimension,
                Node_stack& node_stack, Sample_to& sample) const noexcept override final;

    bool sample(uint32_t part, Transformation const& transformation, float area, bool two_sided,
                sampler::Sampler& sampler, uint32_t sampler_dimension, math::AABB const& bounds,
                Node_stack& node_stack, Sample_from& sample) const noexcept override final;

    float pdf(Ray const& ray, Intersection const& intersection,
              Transformation const& transformation, float area, bool two_sided,
              bool total_sphere) const noexcept override final;

    bool sample(uint32_t part, float3 const& p, float2 uv, Transformation const& transformation,
                float area, bool two_sided, Sample_to& sample) const noexcept override final;

    bool sample(uint32_t part, float2 uv, Transformation const& transformation, float area,
                bool two_sided, sampler::Sampler& sampler, uint32_t sampler_dimension,
                math::AABB const& bounds, Sample_from& sample) const noexcept override final;

    float pdf_uv(Ray const& ray, Intersection const& intersection,
                 Transformation const& transformation, float area, bool two_sided) const
        noexcept override final;

    float uv_weight(float2 uv) const noexcept override final;

    float area(uint32_t part, float3 const& scale) const noexcept override final;

    size_t num_bytes() const noexcept override final;
};

}  // namespace scene::shape

#endif
