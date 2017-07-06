#pragma once

#include "triangle_type.hpp"
#include "scene/shape/shape.hpp"
#include "bvh/triangle_bvh_tree.hpp"
// #include "bvh/triangle_bvh_data.hpp"
#include "bvh/triangle_bvh_indexed_data.hpp"
// #include "bvh/triangle_bvh_data_interleaved.hpp"
#include "base/math/distribution/distribution_1d.hpp"

namespace scene { namespace shape { namespace triangle {

class Mesh : public Shape {

public:

//	using Tree = bvh::Tree<bvh::Data_interleaved<Triangle_type>>;

//	using Tree = bvh::Tree<bvh::Data<Intersection_triangle_type, Shading_triangle_type>>;

	using Tree = bvh::Tree<bvh::Indexed_data<Intersection_vertex_type, Shading_vertex_type>>;

//	using Tree = bvh::Tree<bvh::Indexed_data_interleaved<Vertex_type>>;

	bool init();

	Tree& tree();

	virtual uint32_t num_parts() const override final;

	virtual bool intersect(const Transformation& transformation,
						   Ray& ray, Node_stack& node_stack,
						   shape::Intersection& intersection) const override final;

	virtual bool intersect_p(const Transformation& transformation, const Ray& ray,
							 Node_stack& node_stack) const override final;

//	virtual bool intersect_p(VVector ray_origin, VVector ray_direction,
//							 VVector ray_min_t, VVector ray_max_t,
//							 const Transformation& transformation,
//							 Node_stack& node_stack) const override final;

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
					  const float3& p, const float3& wi, float offset, float area, bool two_sided,
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

	virtual size_t num_bytes() const override final;

private:

    Tree tree_;

	struct Distribution {
		void init(uint32_t part, const Tree& tree);
		bool empty() const;

		uint32_t sample(float r) const;

		size_t num_bytes() const;

		std::vector<uint32_t> triangle_mapping;
		math::Distribution_lut_1D distribution;
	};

	std::vector<Distribution> distributions_;
};

}}}

