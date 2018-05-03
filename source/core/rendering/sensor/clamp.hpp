#pragma once

#include "base/math/vector4.hpp"

namespace rendering::sensor::clamp {

class Identity {

public:

	float4 clamp(float4 const& color) const;
};

class Clamp {

public:

	Clamp(float3 const& max);

	float4 clamp(float4 const& color) const;

private:

	float3 max_;
};

}
