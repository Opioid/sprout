#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HEIGHT_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_HEIGHT_HPP

#include "volumetric_density.hpp"

namespace scene::material::volumetric {

class Height final : public Density {

public:

	Height(const Sampler_settings& sampler_settings);

	void set_a_b(float a, float b);

	virtual size_t num_bytes() const override final;

private:

	virtual float density(const Transformation& transformation, f_float3 p,
						  Sampler_filter filter, const Worker& worker) const override final;

	float a_ = 1.f;
	float b_ = 1.f;
};

}

#endif
