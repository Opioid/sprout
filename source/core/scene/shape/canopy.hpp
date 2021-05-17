#ifndef SU_CORE_SCENE_SHAPE_CANOPY_HPP
#define SU_CORE_SCENE_SHAPE_CANOPY_HPP

#include "shape.hpp"

namespace scene::shape {

class Canopy final : public Shape {
  public:
    Canopy();

    AABB aabb() const final;

    bool intersect(Ray& ray, Transformation const& trafo, Node_stack& nodes,
                   Interpolation ipo, Intersection& isec) const final;

    bool intersect_p(Ray const& ray, Transformation const& trafo, Node_stack& nodes) const final;

    float visibility(Ray const& ray, Transformation const& trafo, uint32_t entity, Filter filter,
                     Worker& worker) const final;

    bool thin_absorption(Ray const& ray, Transformation const& trafo, uint32_t entity,
                         Filter filter, Worker& worker, float3& ta) const final;

    bool sample(uint32_t part, uint32_t variant, float3_p p, float3_p n,
                Transformation const& trafo, float area, bool two_sided, bool total_sphere,
                Sampler& sampler, RNG& rng, uint32_t sampler_d, Sample_to& sample) const final;

    bool sample(uint32_t part, uint32_t variant, Transformation const& trafo, float area,
                bool two_sided, Sampler& sampler, RNG& rng, uint32_t sampler_d,
                float2 importance_uv, AABB const& bounds, Sample_from& sample) const final;

    float pdf(uint32_t variant, Ray const& ray, float3_p n, Intersection const& isec,
              Transformation const& trafo, float area, bool two_sided,
              bool total_sphere) const final;

    float pdf_volume(Ray const& ray, Intersection const& isec, Transformation const& trafo,
                     float area) const final;

    bool sample(uint32_t part, float3_p p, float2 uv, Transformation const& trafo, float area,
                bool two_sided, Sample_to& sample) const final;

    bool sample_volume(uint32_t part, float3_p p, float3_p uvw, Transformation const& trafo,
                       float volume, Sample_to& sample) const final;

    bool sample(uint32_t part, float2 uv, Transformation const& trafo, float area, bool two_sided,
                float2 importance_uv, AABB const& bounds, Sample_from& sample) const final;

    float pdf_uv(Ray const& ray, Intersection const& isec, Transformation const& trafo, float area,
                 bool two_sided) const final;

    float uv_weight(float2 uv) const final;

    float area(uint32_t part, float3_p scale) const final;

    float volume(uint32_t part, float3_p scale) const final;

    Differential_surface differential_surface(uint32_t primitive) const final;
};

}  // namespace scene::shape

#endif
