#pragma once

#include "scene/shape/shape.hpp"
#include "bvh/triangle_bvh_tree.hpp"

namespace scene { namespace shape { namespace triangle {

class Mesh : public Shape {
public:

	virtual bool intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds,
						   shape::Intersection& intersection, float& hit_t) const;
	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds) const;

	virtual void importance_sample(const Composed_transformation& transformation, const math::float3& p, sampler::Sampler& sampler, uint32_t sample_index,
								   math::float3& wi, float& t, float& pdf) const;

	virtual bool is_complex() const;

private:

	bvh::Tree tree_;

	friend class Provider;
};

}}}

