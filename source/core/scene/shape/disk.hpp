#ifndef SU_CORE_SCENE_SHAPE_DISK_HPP
#define SU_CORE_SCENE_SHAPE_DISK_HPP

#include "shape.hpp"

namespace scene::shape {

class Disk final : public Shape {
  public:
    Disk();

    float3 object_to_texture_point(float3 const& p) const final;

    float3 object_to_texture_vector(float3 const& v) const final;

    AABB transformed_aabb(float4x4 const& m) const final;

    bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                   Intersection& intersection) const final;

    bool intersect_nsf(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                       Intersection& intersection) const final;

    bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                   Normals& normals) const final;

    bool intersect_p(Ray const& ray, Transformation const& transformation,
                     Node_stack& node_stack) const final;

    float visibility(Ray const& ray, Transformation const& transformation, uint32_t entity,
                     Filter filter, Worker const& worker) const final;

    bool thin_absorption(Ray const& ray, Transformation const& transformation, uint32_t entity,
                         Filter filter, Worker const& worker, float3& ta) const final;

    bool sample(uint32_t part, float3 const& p, Transformation const& transformation, float area,
                bool two_sided, Sampler& sampler, uint32_t sampler_dimension,
                Sample_to& sample) const final;

    bool sample(uint32_t part, Transformation const& transformation, float area, bool two_sided,
                Sampler& sampler, uint32_t sampler_dimension, float2 importance_uv,
                AABB const& bounds, Sample_from& sample) const final;

    float pdf(Ray const& ray, Intersection const& intersection,
              Transformation const& transformation, float area, bool two_sided,
              bool total_sphere) const final;

    float pdf_volume(Ray const& ray, Intersection const& intersection,
                     Transformation const& transformation, float volume) const final;

    bool sample(uint32_t part, float3 const& p, float2 uv, Transformation const& transformation,
                float area, bool two_sided, Sample_to& sample) const final;

    bool sample(uint32_t part, float3 const& p, float3 const& uvw,
                Transformation const& transformation, float volume, Sample_to& sample) const final;

    bool sample(uint32_t part, float2 uv, Transformation const& transformation, float area,
                bool two_sided, float2 importance_uv, AABB const& bounds,
                Sample_from& sample) const final;

    float pdf_uv(Ray const& ray, Intersection const& intersection,
                 Transformation const& transformation, float area, bool two_sided) const final;

    float uv_weight(float2 uv) const final;

    float area(uint32_t part, float3 const& scale) const final;

    float volume(uint32_t part, float3 const& scale) const final;

    size_t num_bytes() const final;
};

}  // namespace scene::shape

#endif
