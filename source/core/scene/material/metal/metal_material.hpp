#pragma once

#include "scene/material/material.hpp"
#include "scene/material/material_sample_cache.hpp"

namespace scene { namespace material { namespace metal {

class Sample_isotropic;

class Material_isotropic : public material::Typed_material<Generic_sample_cache<Sample_isotropic>> {

public:

	Material_isotropic(Generic_sample_cache<Sample_isotropic>& cache,
					   const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_settings::Filter filter) final override;

	void set_normal_map(const Adapter_2D& normal_map);

	void set_ior(float3_p ior);
	void set_absorption(float3_p absorption);
	void set_roughness(float roughness);

protected:

	Adapter_2D normal_map_;

	float3 ior_;
	float3 absorption_;

	float roughness_;
};

class Sample_anisotropic;

class Material_anisotropic : public material::Typed_material<
		Generic_sample_cache<Sample_anisotropic>> {

public:

	Material_anisotropic(Generic_sample_cache<Sample_anisotropic>& cache,
						 const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_settings::Filter filter) final override;

	void set_normal_map(const Adapter_2D& normal_map);
	void set_direction_map(const Adapter_2D& direction_map);

	void set_ior(float3_p ior);
	void set_absorption(float3_p absorption);
	void set_roughness(float2 roughness);

protected:

	Adapter_2D normal_map_;
	Adapter_2D direction_map_;

	float3 ior_;
	float3 absorption_;

	float2 roughness_;
};

}}}

