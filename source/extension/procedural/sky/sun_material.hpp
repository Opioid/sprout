#pragma once

#include "sky_material_base.hpp"

namespace procedural { namespace sky {

class Sun_material : public Material {

public:

	Sun_material(Model& model);

	virtual const scene::material::Sample& sample(const float3& wo, const scene::Renderstate& rs,
												  scene::Worker& worker,
												  Sampler_filter filter) override final;

	virtual float3 sample_radiance(const float3& wi, float2 uv, float area, float time,
								   scene::Worker& worker,
								   Sampler_filter filter) const override final;

	virtual float3 average_radiance(float area) const override final;

	virtual void prepare_sampling(const scene::shape::Shape& shape, uint32_t part,
								  const Transformation& transformation,
								  float area, bool importance_sampling,
								  thread::Pool& pool) override final;

	virtual size_t num_bytes() const override final;
};

}}
