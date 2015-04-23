#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Celestial_disk : public Shape {
public:

	Celestial_disk();

	virtual bool intersect(const Composed_transformation& transformation, const math::Oray& ray,
						   const math::float2& bounds, Node_stack& node_stack,
						   Intersection& intersection, float& hit_t) const;
	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray,
							 const math::float2& bounds, Node_stack& node_stack) const;

	virtual void importance_sample(const Composed_transformation& transformation, const math::float3& p, sampler::Sampler& sampler, uint32_t sample_index,
								   math::float3& wi, float& t, float& pdf) const;

	virtual bool is_finite() const;
};

}}

