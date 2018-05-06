#ifndef SU_CORE_SCENE_SHAPE_CANOPY_HPP
#define SU_CORE_SCENE_SHAPE_CANOPY_HPP

#include "shape.hpp"

namespace scene::shape {

class Canopy final : public Shape {

public:

	Canopy();

	virtual bool intersect(Ray& ray, Transformation const& transformation,
						   Node_stack& node_stack, Intersection& intersection) const override final;

	virtual bool intersect_fast(Ray& ray, Transformation const& transformation,
								Node_stack& node_stack,
								Intersection& intersection) const override final;

	virtual bool intersect(Ray& ray, Transformation const& transformation,
						   Node_stack& node_stack, float& epsilon) const override final;

	virtual bool intersect_p(Ray const& ray, Transformation const& transformation,
							 Node_stack& node_stack) const override final;

	virtual float opacity(Ray const& ray, Transformation const& transformation,
						  const Materials& materials, Sampler_filter filter,
						  Worker const& worker) const override final;

	virtual float3 thin_absorption(Ray const& ray, Transformation const& transformation,
								   const Materials& materials, Sampler_filter filter,
								   Worker const& worker) const override final;

	virtual bool sample(uint32_t part, f_float3 p, f_float3 n,
						Transformation const& transformation, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual bool sample(uint32_t part, f_float3 p, Transformation const& transformation,
						float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual float pdf(Ray const& ray, const Intersection& intersection,
					  Transformation const& transformation,
					  float area, bool two_sided, bool total_sphere) const override final;

	virtual bool sample(uint32_t part, f_float3 p, float2 uv,
						Transformation const& transformation, float area, bool two_sided,
						Sample& sample) const override final;

	virtual float pdf_uv(Ray const& ray, const Intersection& intersection,
						 Transformation const& transformation,
						 float area, bool two_sided) const override final;

	virtual float uv_weight(float2 uv) const override final;

	virtual float area(uint32_t part, f_float3 scale) const override final;

	virtual bool is_finite() const override final;

	virtual size_t num_bytes() const override final;
};

}

#endif
