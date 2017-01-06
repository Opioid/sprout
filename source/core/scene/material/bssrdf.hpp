#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material {

class BSSRDF {

public:

	float3 optical_depth(float length) const;

	float3_p scattering() const;

	void set(float3_p absorption, float3_p scattering);

private:

	float3 absorption_;
	float3 scattering_;
};

}}
