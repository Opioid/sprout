#pragma once

#include "substitute_base_material.hpp"

namespace scene { namespace material { namespace substitute {

class Material_subsurface : public Material_base {

public:

	Material_subsurface(Sample_cache& sample_cache, const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual const BSSRDF& bssrdf(const Worker& worker) final override;

	virtual bool is_subsurface() const final override;

	virtual size_t num_bytes() const final override;

	void set_absorption(const float3& absorption);
	void set_scattering(const float3& scattering);

private:

	float3 absorption_;
	float3 scattering_;
};

}}}

