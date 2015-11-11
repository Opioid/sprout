#pragma once

#include "scene/material/material.hpp"
#include "base/math/bounding/aabb.hpp"

namespace sampler { class Sampler; }

namespace scene {

namespace entity {

struct Composed_transformation;

}

namespace shape {

struct Intersection;
struct Sample;
class Node_stack;
class Morphable_shape;

class Shape {
public:

	virtual ~Shape();

	const math::aabb& aabb() const;

	virtual uint32_t num_parts() const;

	virtual bool intersect(const entity::Composed_transformation& transformation, math::Oray& ray,
						   Node_stack& node_stack, Intersection& intersection) const = 0;

	virtual bool intersect_p(const entity::Composed_transformation& transformation, const math::Oray& ray,
							 Node_stack& node_stack) const = 0;

	virtual float opacity(const entity::Composed_transformation& transformation, const math::Oray& ray,
						  Node_stack& node_stack, const material::Materials& materials,
						  const image::texture::sampler::Sampler_2D& sampler) const = 0;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, const math::float3& n, bool two_sided, bool total_sphere,
						sampler::Sampler& sampler, Node_stack& node_stack, Sample& sample) const = 0;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, math::float2 uv, Sample& sample) const = 0;

	virtual void sample(uint32_t part, const entity::Composed_transformation& transformation, float area,
						const math::float3& p, const math::float3& wi, Sample& sample) const = 0;

	virtual float pdf(uint32_t part, const entity::Composed_transformation& transformation, float area,
					  const math::float3& p, const math::float3& wi, bool two_sided, bool total_sphere,
					  Node_stack& node_stack) const = 0;

	virtual float area(uint32_t part, const math::float3& scale) const = 0;

	virtual bool is_complex() const;
	virtual bool is_finite() const;
	virtual bool is_analytical() const;

	virtual void prepare_sampling(uint32_t part);

	virtual Morphable_shape* morphable_shape();

protected:

	math::aabb aabb_;
};

}}
