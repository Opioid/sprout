#pragma once

#include "scene/material/material.hpp"

namespace scene { namespace material { namespace metal {

class Material_isotropic : public Material {

public:

	Material_isotropic(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
					   bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_settings::Filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_normal_map(const Texture_adapter& normal_map);

	void set_ior(float3_p ior);
	void set_absorption(float3_p absorption);
	void set_roughness(float roughness);

protected:

	Texture_adapter normal_map_;

	float3 ior_;
	float3 absorption_;

	float roughness_;
};

class Material_anisotropic : public Material {

public:

	Material_anisotropic(Sample_cache& sample_cache, const Sampler_settings& sampler_settings,
						 bool two_sided);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_settings::Filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_normal_map(const Texture_adapter& normal_map);
	void set_direction_map(const Texture_adapter& direction_map);

	void set_ior(float3_p ior);
	void set_absorption(float3_p absorption);
	void set_roughness(float2 roughness);

protected:

	Texture_adapter normal_map_;
	Texture_adapter direction_map_;

	float3 ior_;
	float3 absorption_;

	float2 roughness_;
};

}}}

