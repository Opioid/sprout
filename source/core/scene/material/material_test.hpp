#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material {

namespace bxdf { struct Result; }

namespace testing {

bool check(float3_p result, float3_p wi, float3_p wo, float pdf);

bool check(const bxdf::Result& result, float3_p wo);

}}}
