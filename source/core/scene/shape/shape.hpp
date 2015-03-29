#pragma once

#include "base/math/bounding/aabb.hpp"

namespace sampler {

class Sampler;

}

namespace scene {

struct Composed_transformation;

namespace shape {

struct Intersection;

class Shape {
public:

	virtual ~Shape();

	const math::AABB& aabb() const;

	virtual bool intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounding,
						   Intersection& intersection, float& hit_t) const = 0;
	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounding) const = 0;

	virtual void importance_sample(const Composed_transformation& transformation, const math::float3& p, sampler::Sampler& sampler, uint32_t sample_index,
								   math::float3& wi, float& t, float& pdf) const = 0;

	virtual bool is_complex() const;
	virtual bool is_finite() const;

protected:

	math::AABB aabb_;
};

}}
