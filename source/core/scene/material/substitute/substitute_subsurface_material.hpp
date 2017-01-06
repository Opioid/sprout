#pragma once

#include "substitute_base_material.hpp"

namespace scene { namespace material { namespace substitute {

class Sample_subsurface;

class Material_subsurface : public Material_base<Sample_subsurface> {

public:

	Material_subsurface(BSSRDF_cache& bssrdf_cache, const Sampler_settings& sampler_settings,
						bool two_sided, Sample_cache<Sample_subsurface>& cache);

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	virtual bool is_subsurface() const final override;

	virtual size_t num_bytes() const final override;

	void set_scattering(float3_p scattering);

private:

	float3 scattering_;
};

}}}

