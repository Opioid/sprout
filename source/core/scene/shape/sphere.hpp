#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Sphere : public Shape {

public:

	Sphere();

	virtual math::AABB transformed_aabb(const float4x4& m,
										const math::Transformation& t) const override final;

	virtual math::AABB transformed_aabb(const math::Transformation& t) const override final;

	virtual bool intersect(const Transformation& transformation,
						   Ray& ray, Node_stack& node_stack,
						   Intersection& intersection) const override final;

	virtual bool intersect_p(const Transformation& transformation, const Ray& ray,
							 Node_stack& node_stack) const override final;

	virtual float opacity(const Transformation& transformation, const Ray& ray,
						  const material::Materials& materials, Worker& worker,
						  Sampler_filter filter) const override final;

	virtual float3 thin_absorption(const Transformation& transformation, const Ray& ray,
								   const material::Materials& materials, Worker& worker,
								   Sampler_filter filter) const override final;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, const float3& n, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual float pdf(uint32_t part, const Transformation& transformation,
					  const float3& p, const float3& wi, float offset, float area, bool two_sided,
					  bool total_sphere, Node_stack& node_stack) const override final;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float2 uv, float area, bool two_sided,
						Sample& sample) const override final;

	virtual float pdf_uv(uint32_t part, const Transformation& transformation,
						 const float3& p, const float3& wi, float area, bool two_sided,
						 float2& uv) const override final;

	virtual float uv_weight(float2 uv) const override final;

	virtual float area(uint32_t part, const float3& scale) const override final;

	virtual size_t num_bytes() const override final;
};

}}
