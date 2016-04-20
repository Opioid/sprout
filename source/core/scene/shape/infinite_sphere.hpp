#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Infinite_sphere : public Shape {
public:

	Infinite_sphere();

	virtual bool intersect(const Entity_transformation& transformation, math::Oray& ray,
						   Node_stack& node_stack, Intersection& intersection) const final override;

	virtual bool intersect_p(const Entity_transformation& transformation, const math::Oray& ray,
							 Node_stack& node_stack) const final override;

	virtual float opacity(const Entity_transformation& transformation, const math::Oray& ray,
						  float time, const material::Materials& materials, Worker& worker,
						  Sampler_filter filter) const final override;

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

	virtual bool is_finite() const final override;
};

}}

