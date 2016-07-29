#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Disk : public Shape {

public:

	Disk();

	virtual bool intersect(const Transformation& transformation,
						   math::Oray& ray, Node_stack& node_stack,
						   Intersection& intersection) const final override;

	virtual bool intersect_p(const Transformation& transformation,
							 const math::Oray& ray,
							 Node_stack& node_stack) const final override;

	virtual float opacity(const Transformation& transformation,
						  const math::Oray& ray, float time,
						  const material::Materials& materials,
						  Worker& worker, Sampler_filter filter) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						float3_p p, float3_p n, float area,
						bool two_sided, sampler::Sampler& sampler,
						Node_stack& node_stack, Sample& sample) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						float3_p p, float area, bool two_sided,
						sampler::Sampler& sampler, Node_stack& node_stack,
						Sample& sample) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						float3_p p, float2 uv, float area,
						Sample& sample) const final override;

	virtual void sample(uint32_t part, const Transformation& transformation,
						float3_p p, float3_p wi, float area,
						Sample& sample) const final override;

	virtual float pdf(uint32_t part, const Transformation& transformation,
					  float3_p p, float3_p wi, float area, bool two_sided,
					  bool total_sphere, Node_stack& node_stack) const final override;

	virtual float area(uint32_t part, float3_p scale) const final override;
};

}}

