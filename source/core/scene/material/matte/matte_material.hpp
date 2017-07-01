#pragma once

#include "scene/material/material.hpp"

namespace scene { namespace material { namespace matte {

class Material : public material::Material {

public:

	Material(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Worker& worker, Sampler_filter filter) final override;

	virtual size_t num_bytes() const final override;

	void set_color(const float3& color);

private:

	float3 color_;
};

}}}
