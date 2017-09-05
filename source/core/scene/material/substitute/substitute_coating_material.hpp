#pragma once

#include "substitute_base_material.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace substitute {

template<typename Coating>
class Material_coating : public Material_base {

public:

	Material_coating(const Sampler_settings& sampler_settings, bool two_sided);

	virtual size_t num_bytes() const override final;

	void set_coating_weight_map(const Texture_adapter& weight_map);
	void set_coating_normal_map(const Texture_adapter& normal_map);

	void set_coating_weight(float weight);
	void set_coating_color(const float3& color);

protected:

	template<typename Sample>
	void set_coating_basis(const Renderstate& rs, const Texture_sampler_2D& sampler,
						   Sample& sample);

	Texture_adapter coating_weight_map_;
	Texture_adapter coating_normal_map_;

	Coating coating_;
};

class Material_clearcoat : public Material_coating<coating::Clearcoat> {

public:

	Material_clearcoat(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter, Worker& worker) override final;

	void set_clearcoat(float ior, float roughness);
};

class Material_thinfilm : public Material_coating<coating::Thinfilm> {

public:

	Material_thinfilm(const Sampler_settings& sampler_settings, bool two_sided);

	virtual const material::Sample& sample(const float3& wo, const Renderstate& rs,
										   Sampler_filter filter, Worker& worker) override final;

	void set_thinfilm(float ior, float roughness, float thickness);
};

}}}
