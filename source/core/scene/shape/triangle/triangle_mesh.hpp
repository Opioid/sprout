#pragma once

#include "scene/shape/shape.hpp"
//#include "triangle_distribution.hpp"
#include "bvh/triangle_bvh_tree.hpp"
#include "bvh/triangle_bvh_xtree.hpp"
#include "base/math/cdf.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle;

class Mesh : public Shape {
public:

	void init();

	virtual uint32_t num_parts() const final override;

	virtual bool intersect(const Composed_transformation& transformation, math::Oray& ray,
						   const math::float2& bounds, Node_stack& node_stack,
						   shape::Intersection& intersection) const final override;

	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray,
							 const math::float2& bounds, Node_stack& node_stack) const final override;

	virtual float opacity(const Composed_transformation& transformation, const math::Oray& ray,
						  const math::float2& bounds, Node_stack& node_stack,
						  const material::Materials& materials, const image::sampler::Sampler_2D& sampler) const final override;

	virtual void sample(uint32_t part, const Composed_transformation& transformation, float area, const math::float3& p, const math::float3& n,
						sampler::Sampler& sampler, Sample& sample) const final override;

	virtual float pdf(uint32_t part, const Composed_transformation& transformation, float area,
					  const math::float3& p, const math::float3& wi) const final override;

	virtual float area(uint32_t part, const math::float3& scale) const final override;

	virtual bool is_complex() const final override;

	virtual void prepare_sampling(uint32_t part, const math::float3& scale) final override;

private:

	bvh::XTree tree_;

	struct Distribution {
		void init(uint32_t part, const std::vector<Triangle>& triangles, const math::float3& scale);
		uint32_t sample(float r);

		float area;
		math::CDF cdf;
		std::vector<uint32_t> triangle_mapping;
	};

	std::vector<Distribution> distributions_;

	friend class Provider;
};

}}}

