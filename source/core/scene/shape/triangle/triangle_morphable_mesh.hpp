#pragma once

#include "triangle_type.hpp"
#include "scene/shape/shape.hpp"
#include "scene/shape/morphable_shape.hpp"
#include "bvh/triangle_bvh_tree.hpp"
#include "bvh/triangle_bvh_indexed_data.hpp"
//#include "bvh/triangle_bvh_data_interleaved.hpp"
#include "base/math/distribution/distribution_1d.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_MT;
class  Morph_target_collection;

class Morphable_mesh : public Shape, public Morphable_shape {

public:

	Morphable_mesh(std::shared_ptr<Morph_target_collection> collection, uint32_t num_parts);

	void init();

	virtual uint32_t num_parts() const override final;

	virtual bool intersect(const Transformation& transformation,
						   Ray& ray, Node_stack& node_stack,
						   shape::Intersection& intersection) const override final;

	virtual bool intersect_p(const Transformation& transformation, const Ray& ray,
							 Node_stack& node_stack) const override final;

	virtual float opacity(const Transformation& transformation, const Ray& ray,
						  const material::Materials& materials, Worker& worker,
						  Sampler_filter filter) const override final;

	virtual float3 thin_absorption(const Transformation& transformation, const Ray& ray,
								   const material::Materials& materials, Worker& worker,
								   Sampler_filter filter) const override final;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, const float3& n, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual float pdf(uint32_t part, const Transformation& transformation,
					  const float3& p, const float3& wi, float area, bool two_sided,
					  bool total_sphere, Node_stack& node_stack) const override final;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float2 uv, float area, bool two_sided,
						Sample& sample) const override final;

	virtual float pdf_uv(uint32_t part, const Transformation& transformation,
						 const float3& p, const float3& wi, float area, bool two_sided,
						 float2& uv) const override final;

	virtual float uv_weight(float2 uv) const override final;

	virtual float area(uint32_t part, const float3& scale) const override final;

	virtual bool is_complex() const override final;

	virtual bool is_analytical() const override final;

	virtual void prepare_sampling(uint32_t part) override final;

	virtual Morphable_shape* morphable_shape() override final;

	virtual void morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool) override final;

	virtual size_t num_bytes() const override final;

private:

//	using Tree = bvh::Tree<bvh::Data_interleaved<Triangle_MT>>;

	using Tree = bvh::Tree<bvh::Indexed_data<Intersection_vertex_type, Shading_vertex_type>>;

	Tree tree_;

	std::shared_ptr<Morph_target_collection> collection_;

	std::vector<Vertex> vertices_;

	friend class Provider;
};

}}}

