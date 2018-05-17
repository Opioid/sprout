#pragma once

#include "scene/material/light/light_emissionmap.hpp"

namespace scene::material::display {

class Emissionmap : public light::Emissionmap {

public:

	Emissionmap(Sampler_settings const& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   Worker const& worker) const override final;

	virtual float ior() const override final;

	virtual size_t num_bytes() const override final;

	void set_roughness(float roughness);
	void set_ior(float ior);

private:

	float roughness_;

	float ior_;

	float f0_;
};

}
