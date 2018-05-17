#pragma once

#include "substitute_base_material.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene::material::substitute {

template<typename Coating>
class Material_coating : public Material_base {

public:

	Material_coating(Sampler_settings const& sampler_settings, bool two_sided);

	virtual size_t num_bytes() const override final;

	void set_coating_weight_map(Texture_adapter const& weight_map);
	void set_coating_normal_map(Texture_adapter const& normal_map);

	void set_coating_weight(float weight);
	void set_coating_color(float3 const& color);

protected:

	template<typename Sample>
	void set_coating_basis(f_float3 wo, Renderstate const& rs,
						   Texture_sampler_2D const& sampler, Sample& sample) const;

	Texture_adapter coating_weight_map_;
	Texture_adapter coating_normal_map_;

	Coating coating_;
};

class Material_clearcoat : public Material_coating<coating::Clearcoat> {

public:

	Material_clearcoat(Sampler_settings const& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   Worker const& worker) const override final;

	void set_clearcoat(float ior, float roughness);

	static size_t sample_size();
};

class Material_thinfilm : public Material_coating<coating::Thinfilm> {

public:

	Material_thinfilm(Sampler_settings const& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   Worker const& worker) const override final;

	void set_thinfilm(float ior, float roughness, float thickness);

	static size_t sample_size();
};

}
