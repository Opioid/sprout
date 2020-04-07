#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MORPHABLE_MESH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MORPHABLE_MESH_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "scene/shape/morphable.hpp"
#include "scene/shape/shape.hpp"
#include "triangle_mesh_bvh.hpp"

namespace scene::shape::triangle {

struct Triangle_MT;
class Morph_target_collection;

class alignas(64) Morphable_mesh final : public Shape, public Morphable {
  public:
    Morphable_mesh(Morph_target_collection* collection, uint32_t num_parts);

    ~Morphable_mesh() final;

    float3 object_to_texture_point(float3 const& p) const final;

    float3 object_to_texture_vector(float3 const& v) const final;

    AABB transformed_aabb(float4x4 const& m) const final;

    uint32_t num_parts() const final;

    bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                   shape::Intersection& intersection) const final;

    bool intersect_nsf(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                       shape::Intersection& intersection) const final;

    bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                   Normals& normals) const final;

    bool intersect_p(Ray const& ray, Transformation const& transformation,
                     Node_stack& node_stack) const final;

    float visibility(Ray const& ray, Transformation const& transformation, uint32_t entity,
                     Filter filter, Worker& worker) const final;

    bool thin_absorption(Ray const& ray, Transformation const& transformation, uint32_t entity,
                         Filter filter, Worker& worker, float3& ta) const final;

    bool sample(uint32_t part, float3 const& p, Transformation const& transformation, float area,
                bool two_sided, sampler::Sampler& sampler, uint32_t sampler_dimension,
                Sample_to& sample) const final;

    bool sample(uint32_t part, Transformation const& transformation, float area, bool two_sided,
                sampler::Sampler& sampler, uint32_t sampler_dimension, float2 importance_uv,
                AABB const& bounds, Sample_from& sample) const final;

    float pdf(Ray const& ray, shape::Intersection const& intersection,
              Transformation const& transformation, float area, bool two_sided,
              bool total_sphere) const final;

    float pdf_volume(Ray const& ray, shape::Intersection const& intersection,
                     Transformation const& transformation, float volume) const final;

    bool sample(uint32_t part, float3 const& p, float2 uv, Transformation const& transformation,
                float area, bool two_sided, Sample_to& sample) const final;

    bool sample(uint32_t part, float3 const& p, float3 const& uvw,
                Transformation const& transformation, float volume, Sample_to& sample) const final;

    bool sample(uint32_t part, float2 uv, Transformation const& transformation, float area,
                bool two_sided, float2 importance_uv, AABB const& bounds,
                Sample_from& sample) const final;

    float pdf_uv(Ray const& ray, shape::Intersection const& intersection,
                 Transformation const& transformation, float area, bool two_sided) const final;

    float uv_weight(float2 uv) const final;

    float area(uint32_t part, float3 const& scale) const final;

    float volume(uint32_t part, float3 const& scale) const final;

    bool is_complex() const final;

    bool is_analytical() const final;

    void prepare_sampling(uint32_t part) final;

    Morphable* morphable_shape() final;

    void morph(uint32_t a, uint32_t b, float weight, thread::Pool& threads) final;

    size_t num_bytes() const final;

  private:
    Tree tree_;

    Morph_target_collection* collection_;

    Vertex* vertices_;

    friend class Provider;
};

}  // namespace scene::shape::triangle

#endif
