#pragma once

#include "base/math/vector3.hpp"

namespace scene { namespace material {

class BSSRDF {

public:

	float3 optical_depth(float length) const;

	const float3& scattering() const;

	void set(const float3& absorption, const float3& scattering);

private:

	float3 absorption_;
	float3 scattering_;
};

}}
