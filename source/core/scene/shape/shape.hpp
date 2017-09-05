#pragma once

#include "scene/material/material.hpp"
#include "base/math/aabb.hpp"
#include "base/math/matrix4x4.hpp"
#include "base/math/transformation.hpp"

namespace sampler { class Sampler; }

namespace scene {

struct Ray;
class Worker;

namespace entity { struct Composed_transformation; }

namespace shape {

struct Intersection;
struct Sample;
class Node_stack;
class Morphable_shape;

class Shape {

public:

	using Transformation = entity::Composed_transformation;
	using Sampler_filter = material::Sampler_settings::Filter;

	virtual ~Shape();

	const math::AABB& aabb() const;

	virtual math::AABB transformed_aabb(const float4x4& m, const math::Transformation& t) const;
	virtual math::AABB transformed_aabb(const math::Transformation& t) const;

	virtual uint32_t num_parts() const;

	virtual bool intersect(const Transformation& transformation, Ray& ray,
						   Node_stack& node_stack, Intersection& intersection) const = 0;

	virtual bool intersect_p(const Transformation& transformation,
							 const Ray& ray, Node_stack& node_stack) const = 0;

//	virtual bool intersect_p(VVector ray_origin, VVector ray_direction,
//							 VVector ray_min_t, VVector ray_max_t,
//							 const Transformation& transformation,
//							 Node_stack& node_stack) const;

	virtual float opacity(const Transformation& transformation, const Ray& ray,
						  const material::Materials& materials,
						  Sampler_filter filter, Worker& worker) const = 0;

	virtual float3 thin_absorption(const Transformation& transformation, const Ray& ray,
								   const material::Materials& materials,
								   Sampler_filter filter, Worker& worker) const = 0;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, const float3& n, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const = 0;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const = 0;

	// Both pdf functions implicitely assume that the passed
	// ray/intersection/transformation combination actually lead to a hit.
	virtual float pdf(const Ray& ray, const Intersection& intersection,
					  const Transformation& transformation,
					  float area, bool two_sided, bool total_sphere) const = 0;

	// The following two functions are used for textured lights
	// and should have the uv weight baked in!
	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float2 uv, float area, bool two_sided,
						Sample& sample) const = 0;

	virtual float pdf_uv(const Ray& ray, const Intersection& intersection,
						 const Transformation& transformation,
						 float area, bool two_sided) const = 0;

	virtual float uv_weight(float2 uv) const = 0;

	virtual float area(uint32_t part, const float3& scale) const = 0;

	virtual bool is_complex() const;
	virtual bool is_finite() const;
	virtual bool is_analytical() const;

	virtual void prepare_sampling(uint32_t part);

	virtual Morphable_shape* morphable_shape();

	virtual size_t num_bytes() const = 0;

protected:

	math::AABB aabb_;
};

}}
