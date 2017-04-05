#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Plane : public Shape {

public:

	Plane();

	virtual bool intersect(const Transformation& transformation,
						   Ray& ray, Node_stack& node_stack,
						   Intersection& intersection) const final override;

	virtual bool intersect_p(const Transformation& transformation, const Ray& ray,
							 Node_stack& node_stack) const final override;

	virtual float opacity(const Transformation& transformation, const Ray& ray,
						  const material::Materials& materials, Worker& worker,
						  Sampler_filter filter) const final override;

	virtual float3 thin_absorption(const Transformation& transformation, const Ray& ray,
								   const material::Materials& materials, Worker& worker,
								   Sampler_filter filter) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, const float3& n, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const final override;

	virtual float pdf(uint32_t part, const Transformation& transformation,
					  const float3& p, const float3& wi, float area, bool two_sided,
					  bool total_sphere, Node_stack& node_stack) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float2 uv, float area, bool two_sided,
						Sample& sample) const final override;

	virtual float pdf_uv(uint32_t part, const Transformation& transformation,
						 const float3& p, const float3& wi, float area, bool two_sided,
						 float2& uv) const final override;

	virtual float uv_weight(float2 uv) const final override;

	virtual float area(uint32_t part, const float3& scale) const final override;

	virtual bool is_finite() const final override;

	virtual size_t num_bytes() const final override;
};

}}
