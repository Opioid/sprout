#pragma once

#include "base/math/vector3.hpp"

namespace rendering { namespace sensor { namespace clamp {

class Identity {

public:

	float4 clamp(float4_p color) const;
};

class Clamp {

public:

	Clamp(float3_p max);

	float4 clamp(float4_p color) const;

private:

	float3 max_;
};

}}}
