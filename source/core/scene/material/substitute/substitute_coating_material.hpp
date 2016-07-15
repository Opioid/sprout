#pragma once

#include "substitute_base_material.hpp"
#include "substitute_coating_sample.hpp"
#include "scene/material/coating/coating.hpp"

namespace scene { namespace material { namespace substitute {

template<typename Coating, typename Sample>
class Material_coating : public Material_base<Sample> {

public:

	Material_coating(Generic_sample_cache<Sample>& cache,
					 const Sampler_settings& sampler_settings, bool two_sided);

	void set_coating_weight_map(const Adapter_2D& weight_map);
	void set_coating_normal_map(const Adapter_2D& normal_map);

	void set_coating_weight(float weight);
	void set_coating_color(float3_p color);

protected:

	using Texture_sampler_2D = image::texture::sampler::Sampler_2D;

	void set_coating_basis(const Renderstate& rs, const Texture_sampler_2D& sampler,
						   Sample& sample);

	Adapter_2D coating_weight_map_;
	Adapter_2D coating_normal_map_;

	Coating coating_;
};

class Material_clearcoat : public Material_coating<coating::Clearcoat, Sample_clearcoat> {

public:

	Material_clearcoat(Generic_sample_cache<Sample_clearcoat>& cache,
					   const Sampler_settings& sampler_settings, bool two_sided);

	using Sampler_filter = material::Sampler_settings::Filter;

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	void set_clearcoat(float ior, float roughness);
};

class Material_thinfilm : public Material_coating<coating::Thinfilm, Sample_thinfilm> {

public:

	Material_thinfilm(Generic_sample_cache<Sample_thinfilm>& cache,
					  const Sampler_settings& sampler_settings, bool two_sided);

	using Sampler_filter = material::Sampler_settings::Filter;

	virtual const material::Sample& sample(float3_p wo, const Renderstate& rs,
										   const Worker& worker,
										   Sampler_filter filter) final override;

	void set_thinfilm(float ior, float roughness, float thickness);
};

}}}
