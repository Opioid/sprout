#ifndef SU_CORE_SCENE_VOLUME_HOMOEGENEOUS_HPP
#define SU_CORE_SCENE_VOLUME_HOMOEGENEOUS_HPP

#include "volume.hpp"

namespace scene::volume {

class Homogeneous : public Volume {

public:

	virtual float3 optical_depth(const math::Ray& ray, float step_size, rnd::Generator& rng,
								 Sampler_filter filter, const Worker& worker) const override final;

	virtual float3 scattering(const float3& p, Sampler_filter filter,
							  const Worker& worker) const override final;

private:

	virtual void set_parameter(const std::string& name, const json::Value& value) override final;

};

}

#endif
