#ifndef SU_CORE_SCENE_SHAPE_SHAPE_HPP
#define SU_CORE_SCENE_SHAPE_SHAPE_HPP

#include "scene/material/material.hpp"
#include "base/math/aabb.hpp"
#include "base/math/matrix.hpp"

namespace math { struct Transformation; }

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

	virtual math::AABB transformed_aabb(const float4x4& m, math::Transformation const& t) const;
	virtual math::AABB transformed_aabb(math::Transformation const& t) const;

	virtual uint32_t num_parts() const;

	virtual bool intersect(Ray& ray, Transformation const& transformation,
						   Node_stack& node_stack, Intersection& intersection) const = 0;

	virtual bool intersect_fast(Ray& ray, Transformation const& transformation,
								Node_stack& node_stack, Intersection& intersection) const = 0;

	virtual bool intersect(Ray& ray, Transformation const& transformation,
						   Node_stack& node_stack, float& epsilon) const = 0;

	virtual bool intersect_p(Ray const& ray, Transformation const& transformation,
							 Node_stack& node_stack) const = 0;

//	virtual bool intersect_p(FVector ray_origin, FVector ray_direction,
//							 FVector ray_min_t, FVector ray_max_t,
//							 Transformation const& transformation,
//							 Node_stack& node_stack) const;

	virtual float opacity(Ray const& ray, Transformation const& transformation,
						  const Materials& materials,
						  Sampler_filter filter, Worker const& worker) const = 0;

	virtual float3 thin_absorption(Ray const& ray, Transformation const& transformation,
								   const Materials& materials,
								   Sampler_filter filter, Worker const& worker) const = 0;

	virtual bool sample(uint32_t part, f_float3 p, f_float3 n,
						Transformation const& transformation, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const = 0;

	virtual bool sample(uint32_t part, f_float3 p, Transformation const& transformation,
						float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const = 0;

	// Both pdf functions implicitely assume that the passed
	// ray/intersection/transformation combination actually lead to a hit.
	virtual float pdf(Ray const& ray, const Intersection& intersection,
					  Transformation const& transformation,
					  float area, bool two_sided, bool total_sphere) const = 0;

	// The following two functions are used for textured lights
	// and should have the uv weight baked in!
	virtual bool sample(uint32_t part, f_float3 p, float2 uv,
						Transformation const& transformation, float area,
						bool two_sided, Sample& sample) const = 0;

	virtual float pdf_uv(Ray const& ray, const Intersection& intersection,
						 Transformation const& transformation,
						 float area, bool two_sided) const = 0;

	virtual float uv_weight(float2 uv) const = 0;

	virtual float area(uint32_t part, f_float3 scale) const = 0;

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

#endif
