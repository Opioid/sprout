#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_MORPHABLE_MESH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_MORPHABLE_MESH_HPP

#include "bvh/triangle_bvh_indexed_data.hpp"
#include "bvh/triangle_bvh_tree.hpp"
#include "scene/shape/morphable_shape.hpp"
#include "scene/shape/shape.hpp"
#include "triangle_type.hpp"
//#include "bvh/triangle_bvh_data_interleaved.hpp"
#include "base/math/distribution/distribution_1d.hpp"

namespace scene::shape::triangle {

struct Triangle_MT;
class Morph_target_collection;

class Morphable_mesh : public Shape, public Morphable_shape {
  public:
    Morphable_mesh(std::shared_ptr<Morph_target_collection> collection,
                   uint32_t                                 num_parts) noexcept;

    void init() noexcept;

    uint32_t num_parts() const noexcept override final;

    bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                   shape::Intersection& intersection) const noexcept override final;

    bool intersect_fast(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                        shape::Intersection& intersection) const noexcept override final;

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
                sampler::Sampler& sampler, uint32_t sampler_dimension, AABB const& bounds,
                Node_stack& node_stack, Sample_from& sample) const noexcept override final;

    float pdf(Ray const& ray, const shape::Intersection& intersection,
              Transformation const& transformation, float area, bool two_sided,
              bool total_sphere) const noexcept override final;

    bool sample(uint32_t part, float3 const& p, float2 uv, Transformation const& transformation,
                float area, bool two_sided, Sample_to& sample) const noexcept override final;

    bool sample(uint32_t part, float2 uv, Transformation const& transformation, float area,
                bool two_sided, sampler::Sampler& sampler, uint32_t sampler_dimension,
                AABB const& bounds, Sample_from& sample) const noexcept override final;

    float pdf_uv(Ray const& ray, const shape::Intersection& intersection,
                 Transformation const& transformation, float area, bool two_sided) const
        noexcept override final;

    float uv_weight(float2 uv) const noexcept override final;

    float area(uint32_t part, float3 const& scale) const noexcept override final;

    bool is_complex() const noexcept override final;

    bool is_analytical() const noexcept override final;

    void prepare_sampling(uint32_t part) noexcept override final;

    Morphable_shape* morphable_shape() noexcept override final;

    void morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool) noexcept override final;

    size_t num_bytes() const noexcept override final;

  private:
    //	using Tree = bvh::Tree<bvh::Data_interleaved<Triangle_MT>>;

    using Tree = bvh::Tree<bvh::Indexed_data<Shading_vertex_type>>;

    Tree tree_;

    std::shared_ptr<Morph_target_collection> collection_;

    std::vector<Vertex> vertices_;

    friend class Provider;
};

}  // namespace scene::shape::triangle

#endif
