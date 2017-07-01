#pragma once

#include "sky_material_base.hpp"
#include "base/math/distribution/distribution_2d.hpp"

namespace procedural { namespace sky {

class Sky_material : public Material {

public:

	Sky_material(Model& model);

	virtual const scene::material::Sample& sample(const float3& wo, const scene::Renderstate& rs,
												  scene::Worker& worker,
												  Sampler_filter filter) final override;

	virtual float3 sample_radiance(const float3& wi, float2 uv, float area,
								   float time, scene::Worker& worker,
								   Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual void prepare_sampling(const scene::shape::Shape& shape, uint32_t part,
								  const Transformation& transformation,
								  float area, bool importance_sampling,
								  thread::Pool& pool) final override;

	virtual size_t num_bytes() const final override;
};

class Sky_baked_material : public Material {

public:

	Sky_baked_material(Model& model);
	~Sky_baked_material();

	virtual const scene::material::Sample& sample(const float3& wo, const scene::Renderstate& rs,
												  scene::Worker& worker,
												  Sampler_filter filter) final override;

	virtual float3 sample_radiance(const float3& wi, float2 uv, float area,
								   float time, scene::Worker& worker,
								   Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual bool has_emission_map() const final override;

	virtual float2 radiance_sample(float2 r2, float& pdf) const final override;

	virtual float emission_pdf(float2 uv, scene::Worker& worker,
							   Sampler_filter filter) const final override;

	virtual void prepare_sampling(const scene::shape::Shape& shape, uint32_t part,
								  const Transformation& transformation,
								  float area, bool importance_sampling,
								  thread::Pool& pool) final override;

	virtual size_t num_bytes() const final override;

private:

	static float3 unclipped_canopy_mapping(const Transformation& transformation, float2 uv);

	Texture_adapter emission_map_;

	float3 average_emission_;

	float total_weight_;

	math::Distribution_2D distribution_;
};

}}
