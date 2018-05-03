#pragma once

#include "substitute_base_material.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene::material::substitute {

template<typename Coating>
class Material_coating : public Material_base {

public:

	Material_coating(const Sampler_settings& sampler_settings, bool two_sided);

	virtual size_t num_bytes() const override final;

	void set_coating_weight_map(const Texture_adapter& weight_map);
	void set_coating_normal_map(const Texture_adapter& normal_map);

	void set_coating_weight(float weight);
	void set_coating_color(float3 const& color);

protected:

	template<typename Sample>
	void set_coating_basis(f_float3 wo, const Renderstate& rs,
						   const Texture_sampler_2D& sampler, Sample& sample) const;

	Texture_adapter coating_weight_map_;
	Texture_adapter coating_normal_map_;

	Coating coating_;
};

class Material_clearcoat : public Material_coating<coating::Clearcoat> {

public:

	Material_clearcoat(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(f_float3 wo, const Renderstate& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   const Worker& worker) const override final;

	void set_clearcoat(float ior, float roughness);

	static size_t sample_size();
};

class Material_thinfilm : public Material_coating<coating::Thinfilm> {

public:

	Material_thinfilm(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(f_float3 wo, const Renderstate& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   const Worker& worker) const override final;

	void set_thinfilm(float ior, float roughness, float thickness);

	static size_t sample_size();
};

}
