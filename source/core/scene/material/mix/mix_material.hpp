#pragma once

#include "scene/material/material.hpp"

namespace scene::material::mix {

class Material : public material::Material {

public:

	Material(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
	                                       Sampler_filter filter, sampler::Sampler& sampler,
										   const Worker& worker) const override final;

	virtual float opacity(float2 uv, float time, Sampler_filter filter,
						  const Worker& worker) const override final;

	virtual bool is_masked() const override final;

	virtual size_t num_bytes() const override final;

	void set_materials(const Material_ptr& a, const Material_ptr& b);

private:

	Material_ptr material_a_;
	Material_ptr material_b_;
};

}
