#pragma once

#include "core/scene/material/material.hpp"
#include "core/scene/material/material_sample_cache.hpp"
#include "core/scene/material/light/light_material_sample.hpp"

namespace procedural { namespace sky {

class Model;

class Material : public scene::material::Typed_material<
		scene::material::Generic_sample_cache<scene::material::light::Sample>> {

public:

	using Sampler_filter = scene::material::Sampler_settings::Filter;

	Material(scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
			 Model& model);

	virtual bool has_emission_map() const final override;

protected:

	Model& model_;
};

class Sky_material : public Material {

public:

	Sky_material(scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
				 Model& model);

	virtual const scene::material::Sample& sample(float3_p wo, const scene::Renderstate& rs,
												  const scene::Worker& worker,
												  Sampler_filter filter) final override;

	virtual float3 sample_radiance(float3_p wi, float2 uv, float area,
								   float time, const scene::Worker& worker,
								   Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual void prepare_sampling(bool spherical) final override;

	virtual size_t num_bytes() const final override;
};

class Sun_material : public Material {

public:

	Sun_material(scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
				 Model& model);

	virtual const scene::material::Sample& sample(float3_p wo, const scene::Renderstate& rs,
												  const scene::Worker& worker,
												  Sampler_filter filter) final override;

	virtual float3 sample_radiance(float3_p wi, float2 uv,
								   float area, float time, const scene::Worker& worker,
								   Sampler_filter filter) const final override;

	virtual float3 average_radiance(float area) const final override;

	virtual void prepare_sampling(bool spherical) final override;

	virtual size_t num_bytes() const final override;
};

}}
