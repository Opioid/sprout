#pragma once

#include "volume.hpp"

namespace scene { namespace volume {

class Homogeneous : public Volume {
public:

	Homogeneous(const float3& absorption, const float3& scattering);

	virtual float3 optical_depth(const math::Oray& ray) const final override;

	virtual float3 scattering() const final override;

	virtual float phase(const float3& w, const float3& wp) const final override;

private:

	float3 absorption_;
	float3 scattering_;
};

}}
