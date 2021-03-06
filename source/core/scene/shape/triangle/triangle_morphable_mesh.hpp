#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MORPHABLE_MESH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MORPHABLE_MESH_HPP

#include "base/math/distribution_1d.hpp"
#include "bvh/triangle_bvh_tree.hpp"
#include "scene/shape/morphable.hpp"
#include "scene/shape/shape.hpp"
#include "triangle_morph_target_collection.hpp"

namespace scene::shape {

struct Vertex;

namespace triangle {

class alignas(64) Morphable_mesh final : public Shape, public Morphable {
  public:
    Morphable_mesh(Morph_target_collection&& collection, uint32_t num_parts);

    ~Morphable_mesh() final;

    AABB aabb() const final;

    uint32_t num_parts() const final;

    bool intersect(Ray& ray, Transformation const& trafo, Worker& worker, Interpolation ipo,
                   shape::Intersection& isec) const final;

    bool intersect_p(Ray const& ray, Transformation const& trafo, Worker& worker) const final;

    bool visibility(Ray const& ray, Transformation const& trafo, uint32_t entity, Filter filter,
                    Worker& worker, float3& v) const final;

    bool sample(uint32_t part, uint32_t variant, float3_p p, float3_p n,
                Transformation const& trafo, float area, bool two_sided, bool total_sphere,
                sampler::Sampler& sampler, RNG& rng, uint32_t sampler_d,
                Sample_to& sample) const final;

    bool sample(uint32_t part, uint32_t variant, Transformation const& trafo, float area,
                bool two_sided, sampler::Sampler& sampler, RNG& rng, uint32_t sampler_d,
                float2 importance_uv, AABB const& bounds, Sample_from& sample) const final;

    float pdf(uint32_t variant, Ray const& ray, float3_p n, shape::Intersection const& isec,
              Transformation const& trafo, float area, bool two_sided,
              bool total_sphere) const final;

    float pdf_volume(Ray const& ray, shape::Intersection const& isec, Transformation const& trafo,
                     float volume) const final;

    bool sample(uint32_t part, float3_p p, float2 uv, Transformation const& trafo, float area,
                bool two_sided, Sample_to& sample) const final;

    bool sample_volume(uint32_t part, float3_p p, float3_p uvw, Transformation const& trafo,
                       float volume, Sample_to& sample) const final;

    bool sample(uint32_t part, float2 uv, Transformation const& trafo, float area, bool two_sided,
                float2 importance_uv, AABB const& bounds, Sample_from& sample) const final;

    float pdf_uv(Ray const& ray, shape::Intersection const& isec, Transformation const& trafo,
                 float area, bool two_sided) const final;

    float uv_weight(float2 uv) const final;

    float area(uint32_t part, float3_p scale) const final;

    float volume(uint32_t part, float3_p scale) const final;

    Differential_surface differential_surface(uint32_t primitive) const final;

    Morphable* morphable_shape() final;

    void morph(Morphing const* morphings, uint32_t num_frames, Threads& threads) final;

  private:
    bvh::Tree tree_;

    Morph_target_collection collection_;

    Vertex* vertices_;

    friend class Provider;
};

}  // namespace triangle

}  // namespace scene::shape

#endif
