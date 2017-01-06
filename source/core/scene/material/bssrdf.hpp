#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material {

class BSSRDF {

public:

	float3_p scattering() const;

	void set(float3_p scattering);

private:

	float3 scattering_;
};

}}
