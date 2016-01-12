#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Homogeneous : public Volume {
public:

	Homogeneous(const math::float3& absorption);

	virtual math::float3 optical_depth(const math::Oray& ray) const final override;

private:

	math::float3 absorption_;
	math::float3 scattering_;
};

}}
