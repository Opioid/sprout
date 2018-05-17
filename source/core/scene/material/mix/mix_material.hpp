#pragma once

#include "scene/material/material.hpp"

namespace scene::material::mix {

class Material : public material::Material {

public:

	Material(Sampler_settings const& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
	                                       Sampler_filter filter, sampler::Sampler& sampler,
										   Worker const& worker) const override final;

	virtual float opacity(float2 uv, float time, Sampler_filter filter,
						  Worker const& worker) const override final;

	virtual bool is_masked() const override final;

	virtual float ior() const override final;

	virtual size_t num_bytes() const override final;

	void set_materials(Material_ptr const& a, Material_ptr const& b);

private:

	Material_ptr material_a_;
	Material_ptr material_b_;
};

}
