#pragma once

#include "scene/material/material.hpp"

namespace scene::material::matte {

class Material : public material::Material {

public:

	Material(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter,
										   const Worker& worker) const override final;

	virtual size_t num_bytes() const override final;

	void set_color(const float3& color);

private:

	float3 color_;
};

}
