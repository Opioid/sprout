#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Disk : public Shape {
public:

	Disk();

	virtual bool intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
						   Node_stack& node_stack, Intersection& intersection) const final override;

	virtual bool intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
							 Node_stack& node_stack) const final override;

	virtual float opacity(const entity::Composed_transformation& transformation, const math::Oray& ray, float time,
						  Node_stack& node_stack, const material::Materials& materials,
						  const image::texture::sampler::Sampler_2D& sampler) const final override;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, const math::float3& n, bool two_sided,
						sampler::Sampler& sampler, Node_stack& node_stack, Sample& sample) const final override;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, bool two_sided,
						sampler::Sampler& sampler, Node_stack& node_stack, Sample& sample) const final override;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, math::float2 uv, Sample& sample) const final override;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, const math::float3& wi, Sample& sample) const final override;

	virtual float pdf(uint32_t part, const entity::Composed_transformation& transformation, float area,
					  const math::float3& p, const math::float3& wi, bool two_sided, bool total_sphere,
					  Node_stack& node_stack) const final override;

	virtual float area(uint32_t part, const math::float3& scale) const final override;
};

}}

