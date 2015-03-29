#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Plane : public Shape {
public:

	Plane();

	virtual bool intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds,
						   Intersection& intersection, float& hit_t) const;
	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds) const;

	virtual void importance_sample(const Composed_transformation& transformation, const math::float3& p, sampler::Sampler& sampler, uint32_t sample_index,
								   math::float3& wi, float& t, float& pdf) const;

	virtual bool is_finite() const;
};

}}
