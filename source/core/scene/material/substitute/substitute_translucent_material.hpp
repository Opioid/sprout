#pragma once

#include "substitute_base_material.hpp"

namespace scene { namespace material { namespace substitute {

class Material_translucent : public Material_base {

public:

	Material_translucent(const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_thickness(float thickness);
	void set_attenuation_distance(float attenuation_distance);

private:

	float thickness_;
	float attenuation_distance_;
};

}}}
