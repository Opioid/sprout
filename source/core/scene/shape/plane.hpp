#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Plane : public Shape {

public:

	Plane();

	virtual bool intersect(const Transformation& transformation,
						   Ray& ray, Node_stack& node_stack,
						   Intersection& intersection) const override final;

	virtual bool intersect_p(const Transformation& transformation, const Ray& ray,
							 Node_stack& node_stack) const override final;

	virtual float opacity(const Transformation& transformation, const Ray& ray,
						  const material::Materials& materials, Sampler_filter filter,
						  Worker& worker) const override final;

	virtual float3 thin_absorption(const Transformation& transformation, const Ray& ray,
								   const material::Materials& materials, Sampler_filter filter,
								   Worker& worker) const override final;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, const float3& n, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual float pdf(const Ray& ray, const Intersection& intersection,
					  const Transformation& transformation,
					  float area, bool two_sided, bool total_sphere) const override final;

	virtual void sample(uint32_t part, const Transformation& transformation,
						const float3& p, float2 uv, float area, bool two_sided,
						Sample& sample) const override final;

	virtual float pdf_uv(const Ray& ray, const Intersection& intersection,
						 const Transformation& transformation,
						 float area, bool two_sided) const override final;

	virtual float uv_weight(float2 uv) const override final;

	virtual float area(uint32_t part, const float3& scale) const override final;

	virtual bool is_finite() const override final;

	virtual size_t num_bytes() const override final;
};

}}
