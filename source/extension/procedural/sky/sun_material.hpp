#pragma once

#include "sky_material_base.hpp"

namespace procedural { namespace sky {

class Sun_material : public Material {

public:

	Sun_material(scene::material::Sample_cache& sample_cache, Model& model);

	virtual const scene::material::Sample& sample(const float3& wo, const scene::Renderstate& rs,
												  const scene::Worker& worker,
												  Sampler_filter filter) final override;

	virtual float3 sample_radiance(const float3& wi, float2 uv,
								   float area, float time, const scene::Worker& worker,
								   Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual void prepare_sampling(const scene::shape::Shape& shape, uint32_t part,
								  const Transformation& transformation,
								  float area, bool importance_sampling,
								  thread::Pool& pool) final override;

	virtual size_t num_bytes() const final override;
};

}}
