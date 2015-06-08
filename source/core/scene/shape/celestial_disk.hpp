#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Celestial_disk : public Shape {
public:

	Celestial_disk();

	virtual bool intersect(const Composed_transformation& transformation, math::Oray& ray,
						   const math::float2& bounds, Node_stack& node_stack,
						   Intersection& intersection) const final override;

	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray,
							 const math::float2& bounds, Node_stack& node_stack) const final override;

	virtual float opacity(const Composed_transformation& transformation, const math::Oray& ray,
						  const math::float2& bounds, Node_stack& node_stack,
						  const material::Materials& materials, const image::sampler::Sampler_2D& sampler) const final override;

	virtual void importance_sample(uint32_t part, const Composed_transformation& transformation, float area, const math::float3& p,
								   sampler::Sampler& sampler, Sample& sample) const final override;

	virtual float pdf(uint32_t part, const Composed_transformation& transformation, float area,
					  const math::float3& p, const math::float3& wi) const final override;

	virtual float area(uint32_t part, const math::float3& scale) const final override;

	virtual bool is_finite() const final override;
};

}}

