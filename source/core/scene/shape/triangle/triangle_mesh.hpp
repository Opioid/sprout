#pragma once

#include "triangle_type.hpp"
#include "scene/shape/shape.hpp"
#include "bvh/triangle_bvh_tree.hpp"
#include "bvh/triangle_bvh_data.hpp"
#include "bvh/triangle_bvh_indexed_data.hpp"
#include "bvh/triangle_bvh_data_interleaved.hpp"
#include "base/math/distribution/distribution_1d.hpp"

namespace scene { namespace shape { namespace triangle {

class Mesh : public Shape {
public:

//	using Tree = bvh::Tree<bvh::Data_interleaved<Triangle_type>>;

//	using Tree = bvh::Tree<bvh::Data<Intersection_triangle_type, Shading_triangle_type>>;

//	using Tree = bvh::Tree<bvh::Indexed_data<Intersection_vertex_type, Shading_vertex_type>>;

	using Tree = bvh::Tree<bvh::Indexed_data1<Intersection_vertex_type, Shading_vertex_type>>;

	bool init();

	Tree& tree();

	virtual uint32_t num_parts() const final override;

	virtual bool intersect(const Entity_transformation& transformation, math::Oray& ray,
						   Node_stack& node_stack,
						   shape::Intersection& intersection) const final override;

	virtual bool intersect_p(const Entity_transformation& transformation, const math::Oray& ray,
							 Node_stack& node_stack) const final override;

	virtual float opacity(const Entity_transformation& transformation, const math::Oray& ray,
						  float time, const material::Materials& materials, Worker& worker,
						  material::Sampler_settings::Filter filter) const final override;

	virtual void sample(uint32_t part, const Entity_transformation& transformation,
						float area, const math::float3& p, const math::float3& n,
						bool two_sided, sampler::Sampler& sampler,
						Node_stack& node_stack, Sample& sample) const final override;

	virtual void sample(uint32_t part, const Entity_transformation& transformation,
						float area, const math::float3& p,
						bool two_sided, sampler::Sampler& sampler,
						Node_stack& node_stack, Sample& sample) const final override;

	virtual void sample(uint32_t part, const Entity_transformation& transformation,
						float area, const math::float3& p,
						math::float2 uv, Sample& sample) const final override;

	virtual void sample(uint32_t part, const Entity_transformation& transformation,
						float area,  const math::float3& p,
						const math::float3& wi, Sample& sample) const final override;

	virtual float pdf(uint32_t part, const Entity_transformation& transformation,
					  float area, const math::float3& p, const math::float3& wi, bool two_sided,
					  bool total_sphere, Node_stack& node_stack) const final override;

	virtual float area(uint32_t part, const math::float3& scale) const final override;

	virtual bool is_complex() const final override;

	virtual bool is_analytical() const final override;

	virtual void prepare_sampling(uint32_t part) final override;

private:

    Tree tree_;

	struct Distribution {
		void init(uint32_t part, const Tree& tree);
		bool empty() const;

		uint32_t sample(float r) const;

		std::vector<uint32_t> triangle_mapping;
		math::Distribution_lut_1D distribution;
	};

	std::vector<Distribution> distributions_;
};

}}}

