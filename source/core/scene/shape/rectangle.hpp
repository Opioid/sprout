#ifndef SU_CORE_SCENE_SHAPE_RECTANGLE_HPP
#define SU_CORE_SCENE_SHAPE_RECTANGLE_HPP

#include "shape.hpp"

namespace scene::shape {

class Rectangle final : public Shape {

public:

	Rectangle();

	virtual bool intersect(Ray& ray, const Transformation& transformation,
						   Node_stack& node_stack, Intersection& intersection) const override final;

	virtual bool intersect_fast(Ray& ray, const Transformation& transformation,
								Node_stack& node_stack,
								Intersection& intersection) const override final;

	virtual bool intersect(Ray& ray, const Transformation& transformation,
						   Node_stack& node_stack, float& epsilon) const override final;

	virtual bool intersect_p(Ray const& ray, const Transformation& transformation,
							 Node_stack& node_stack) const override final;

	virtual float opacity(Ray const& ray, const Transformation& transformation,
						  const Materials& materials, Sampler_filter filter,
						  const Worker& worker) const override final;

	virtual float3 thin_absorption(Ray const& ray, const Transformation& transformation,
								   const Materials& materials, Sampler_filter filter,
								   const Worker& worker) const override final;

	virtual bool sample(uint32_t part, f_float3 p, f_float3 n,
						const Transformation& transformation, float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual bool sample(uint32_t part, f_float3 p, const Transformation& transformation,
						float area, bool two_sided,
						sampler::Sampler& sampler, uint32_t sampler_dimension,
						Node_stack& node_stack, Sample& sample) const override final;

	virtual float pdf(Ray const& ray, const Intersection& intersection,
					  const Transformation& transformation,
					  float area, bool two_sided, bool total_sphere) const override final;

	virtual bool sample(uint32_t part, f_float3 p, float2 uv,
						const Transformation& transformation, float area, bool two_sided,
						Sample& sample) const override final;

	virtual float pdf_uv(Ray const& ray, const Intersection& intersection,
						 const Transformation& transformation,
						 float area, bool two_sided) const override final;

	virtual float uv_weight(float2 uv) const override final;

	virtual float area(uint32_t part, f_float3 scale) const override final;

	virtual size_t num_bytes() const override final;
};

}

#endif
