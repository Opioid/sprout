#pragma once

#include "node_stack.hpp"
#include "scene/material/material.hpp"
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

	const math::aabb& aabb() const;

	virtual uint32_t num_parts() const;

	virtual bool intersect(const Composed_transformation& transformation, math::Oray& ray,
						   const math::float2& bounds, Node_stack& node_stack,
						   Intersection& intersection) const = 0;

	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray,
							 const math::float2& bounds, Node_stack& node_stack) const = 0;

	virtual float opacity(const Composed_transformation& transformation, const math::Oray& ray,
						  const math::float2& bounds, Node_stack& node_stack,
						  const material::Materials& materials, const image::sampler::Sampler_2D& sampler) const = 0;

	virtual void importance_sample(uint32_t part, const Composed_transformation& transformation, float area, const math::float3& p,
								   sampler::Sampler& sampler, uint32_t sample_index,
								   math::float3& wi, float& t, float& pdf) const = 0;

	virtual float pdf(uint32_t part, const Composed_transformation& transformation, float area,
					  const math::float3& p, const math::float3& wi) const = 0;

	virtual float area(uint32_t part, const math::float3& scale) const = 0;

	virtual bool is_complex() const;
	virtual bool is_finite() const;

	virtual void prepare_sampling(uint32_t part, const math::float3& scale);

protected:

	math::aabb aabb_;
};

}}
