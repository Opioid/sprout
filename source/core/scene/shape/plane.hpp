#ifndef SU_CORE_SCENE_SHAPE_PLANE_HPP
#define SU_CORE_SCENE_SHAPE_PLANE_HPP

#include "shape.hpp"

namespace scene::shape {

class Plane final : public Shape {
  public:
    Plane();

    float3 object_to_texture_point(float3 const& p) const final;

    float3 object_to_texture_vector(float3 const& v) const final;

    AABB transformed_aabb(float4x4 const& m) const final;

    bool intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                   Intersection& isec) const final;

    bool intersect_nsf(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                       Intersection& isec) const final;

    bool intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                   Normals& normals) const final;

    bool intersect_p(Ray const& ray, Transformation const& trafo, Node_stack& nodes) const final;

    float visibility(Ray const& ray, Transformation const& trafo, uint32_t entity, Filter filter,
                     Worker& worker) const final;

    bool thin_absorption(Ray const& ray, Transformation const& trafo, uint32_t entity,
                         Filter filter, Worker& worker, float3& ta) const final;

    bool sample(uint32_t part, float3 const& p, Transformation const& trafo, float area,
                bool two_sided, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                Sample_to& sample) const final;

    bool sample(uint32_t part, Transformation const& trafo, float area, bool two_sided,
                Sampler& sampler, RNG& rng, uint32_t sampler_d, float2 importance_uv,
                AABB const& bounds, Sample_from& sample) const final;

    float pdf(Ray const& ray, Intersection const& isec, Transformation const& trafo, float area,
              bool two_sided, bool total_sphere) const final;

    float pdf_volume(Ray const& ray, Intersection const& isec, Transformation const& trafo,
                     float volume) const final;

    bool sample(uint32_t part, float3 const& p, float2 uv, Transformation const& trafo, float area,
                bool two_sided, Sample_to& sample) const final;

    bool sample(uint32_t part, float3 const& p, float3 const& uvw, Transformation const& trafo,
                float volume, Sample_to& sample) const final;

    bool sample(uint32_t part, float2 uv, Transformation const& trafo, float area, bool two_sided,
                float2 importance_uv, AABB const& bounds, Sample_from& sample) const final;

    float pdf_uv(Ray const& ray, Intersection const& isec, Transformation const& trafo, float area,
                 bool two_sided) const final;

    float uv_weight(float2 uv) const final;

    float area(uint32_t part, float3 const& scale) const final;

    float volume(uint32_t part, float3 const& scale) const final;

    Differential_surface differential_surface(uint32_t primitive) const final;
};

}  // namespace scene::shape

#endif
