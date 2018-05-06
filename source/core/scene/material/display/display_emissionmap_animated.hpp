#pragma once

#include "scene/material/light/light_emissionmap_animated.hpp"
#include "image/texture/texture.hpp"
#include "base/math/distribution/distribution_2d.hpp"

namespace scene::material::display {

class Emissionmap_animated : public light::Emissionmap_animated {

public:

	Emissionmap_animated(Sampler_settings const& sampler_settings,
						 bool two_sided, Texture_adapter const& emission_map,
						 float emission_factor, float animation_duration);

	virtual const material::Sample& sample(f_float3 wo, const Renderstate& rs,
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
