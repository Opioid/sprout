#pragma once

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class Sample;

class Material : public Material_base {

public:

	Material(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter,
										   const Worker& worker) override final;

	virtual size_t num_bytes() const override final;
};

}
