#ifndef SU_CORE_SCENE_VOLUME_DENSITY_HPP
#define SU_CORE_SCENE_VOLUME_DENSITY_HPP

#include "volume.hpp"

namespace scene::volume {

class Density : public Volume {

public:

	virtual float3 emission(const Transformation& transformation, const math::Ray& ray,
							float step_size, rnd::Generator& rng,
							Sampler_filter filter, const Worker& worker) const override;

	virtual float3 optical_depth(const Transformation& transformation, const math::Ray& ray,
								 float step_size, rnd::Generator& rng,
								 Sampler_filter filter, const Worker& worker) const override;

	virtual float3 scattering(const Transformation& transformation, const float3& p,
							  Sampler_filter filter, const Worker& worker) const override final;

protected:

	// expects p in object space!
	virtual float density(const Transformation& transformation, const float3& p,
						  Sampler_filter filter, const Worker& worker) const = 0;

private:

};

}

#endif
