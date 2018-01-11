#pragma once

#include "scene/material/material.hpp"

namespace scene::material::debug {

class Material : public material::Material {

public:

	Material(const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   const Worker& worker) const override final;

	virtual size_t num_bytes() const override final;

private:

};

}
