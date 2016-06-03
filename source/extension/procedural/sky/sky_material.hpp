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

	virtual void prepare_sampling(bool spherical) final override;

protected:

	Model& model_;
};

class Sky_material : public Material {

public:

	Sky_material(scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
				 Model& model);

	virtual const scene::material::Sample& sample(const scene::shape::Hitpoint& hp,
												  math::pfloat3 wo, float area, float time,
												  float ior_i, const scene::Worker& worker,
												  Sampler_filter filter) final override;

	virtual math::float3 sample_radiance(math::pfloat3 wi, math::float2 uv,
										 float area, float time, const scene::Worker& worker,
										 Sampler_filter filter) const final override;

	virtual math::float3 average_radiance(float area) const final override;
};

class Sun_material : public Material {

public:

	Sun_material(scene::material::Generic_sample_cache<scene::material::light::Sample>& cache,
				 Model& model);

	virtual const scene::material::Sample& sample(const scene::shape::Hitpoint& hp,
												  math::pfloat3 wo, float area, float time,
												  float ior_i, const scene::Worker& worker,
												  Sampler_filter filter) final override;

	virtual math::float3 sample_radiance(math::pfloat3 wi, math::float2 uv,
										 float area, float time, const scene::Worker& worker,
										 Sampler_filter filter) const final override;

	virtual math::float3 average_radiance(float area) const final override;
};

}}
