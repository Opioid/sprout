#pragma once

#include "math/vector.hpp"

namespace spectrum {

math::float3 blackbody(float temperature);

// defined for [1667, 25000] Kelvin
math::float3 blackbody_fast(float temperature);

}
