#pragma once

#include "scene/material/light/light_emissionmap.hpp"

namespace scene { namespace material { namespace display {

class Emissionmap : public light::Emissionmap {

public:

	Emissionmap(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_roughness(float roughness);
	void set_ior(float ior);

private:

	float roughness_;

	float f0_;
};

}}}
