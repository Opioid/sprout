#ifndef SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_MATERIAL_HPP

#include "glass_material.hpp"

namespace scene::material::glass {

class Glass_dispersion final : public Glass {

public:

	Glass_dispersion(const Sampler_settings& sampler_settings);

	virtual const material::Sample& sample(f_float3 wo, const Renderstate& rs,
										   Sampler_filter filter, sampler::Sampler& sampler,
										   const Worker& worker) const override final;

	virtual size_t num_bytes() const override final;

	void set_abbe(float abbe);

	static size_t sample_size();

private:

	float abbe_;
};

}

#endif

