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
    Morphable_mesh(std::shared_ptr<Morph_target_collection> collection, uint32_t num_parts);

    void init();

    virtual uint32_t num_parts() const override final;

    virtual bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                           shape::Intersection& intersection) const override final;

    virtual bool intersect_fast(Ray& ray, Transformation const& transformation,
                                Node_stack&          node_stack,
                                shape::Intersection& intersection) const override final;

    virtual bool intersect(Ray& ray, Transformation const& transformation, Node_stack& node_stack,
                           float& epsilon) const override final;

    virtual bool intersect_p(Ray const& ray, Transformation const& transformation,
                             Node_stack& node_stack) const override final;

    virtual float opacity(Ray const& ray, Transformation const& transformation,
                          Materials const& materials, Sampler_filter filter,
                          Worker const& worker) const override final;

    virtual float3 thin_absorption(Ray const& ray, Transformation const& transformation,
                                   Materials const& materials, Sampler_filter filter,
                                   Worker const& worker) const override final;

    virtual bool sample(uint32_t part, f_float3 p, f_float3 n, Transformation const& transformation,
                        float area, bool two_sided, sampler::Sampler& sampler,
                        uint32_t sampler_dimension, Node_stack& node_stack,
                        Sample_to& sample) const override final;

    virtual bool sample(uint32_t part, f_float3 p, Transformation const& transformation, float area,
                        bool two_sided, sampler::Sampler& sampler, uint32_t sampler_dimension,
                        Node_stack& node_stack, Sample_to& sample) const override final;

    virtual bool sample(uint32_t part, Transformation const& transformation, float area,
                        bool two_sided, sampler::Sampler& sampler, uint32_t sampler_dimension,
                        Node_stack& node_stack, Sample_from& sample) const override final;

    virtual float pdf(Ray const& ray, const shape::Intersection& intersection,
                      Transformation const& transformation, float area, bool two_sided,
                      bool total_sphere) const override final;

    virtual bool sample(uint32_t part, f_float3 p, float2 uv, Transformation const& transformation,
                        float area, bool two_sided, Sample_to& sample) const override final;

    virtual bool sample(uint32_t part, float2 uv, Transformation const& transformation, float area,
                        bool two_sided, sampler::Sampler& sampler, uint32_t sampler_dimension,
                        Sample_from& sample) const override final;

    virtual float pdf_uv(Ray const& ray, const shape::Intersection& intersection,
                         Transformation const& transformation, float area,
                         bool two_sided) const override final;

    virtual float uv_weight(float2 uv) const override final;

    virtual float area(uint32_t part, f_float3 scale) const override final;

    virtual bool is_complex() const override final;

    virtual bool is_analytical() const override final;

    virtual void prepare_sampling(uint32_t part) override final;

    virtual Morphable_shape* morphable_shape() override final;

    virtual void morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool) override final;

    virtual size_t num_bytes() const override final;

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
