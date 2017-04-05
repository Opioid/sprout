#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Homogeneous : public Volume {

public:

	virtual float3 optical_depth(const math::Ray& ray, float step_size,
								 rnd::Generator& rng, Worker& worker,
								 Sampler_filter filter) const final override;

	virtual float3 scattering(const float3& p, Worker& worker,
							  Sampler_filter filter) const final override;

private:

	virtual void set_parameter(const std::string& name,
							   const json::Value& value) final override;

};

}}
