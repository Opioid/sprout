#pragma once

#include "base/math/vector.hpp"

namespace rendering { namespace sensor { namespace clamp {

class Identity {
public:

	math::float4 clamp(const math::float4& color) const;
};

class Clamp {
public:

	Clamp(const math::float3& max);

	math::float4 clamp(const math::float4& color) const;

private:

	math::float3 max_;
};

}}}
