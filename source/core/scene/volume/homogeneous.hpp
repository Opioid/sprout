#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Homogeneous : public Volume {

public:

	virtual float3 optical_depth(const math::Oray& ray, float step_size,
								 math::random::Generator& rng) const final override;

	virtual float3 scattering(float3_p p) const final override;

private:

	virtual void set_parameter(const std::string& name,
							   const json::Value& value) final override;

};

}}
