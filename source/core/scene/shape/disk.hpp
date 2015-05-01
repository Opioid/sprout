#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Disk : public Shape {
public:

	Disk();

	virtual bool intersect(const Composed_transformation& transformation, const math::Oray& ray,
						   const math::float2& bounds, Node_stack& node_stack,
						   Intersection& intersection, float& hit_t) const final override;

	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray,
							 const math::float2& bounds, Node_stack& node_stack) const final override;

	virtual void importance_sample(const Composed_transformation& transformation, const math::float3& p, sampler::Sampler& sampler, uint32_t sample_index,
								   math::float3& wi, float& t, float& pdf) const final override;

	virtual float area(const math::float3& scale) const final override;
};

}}

