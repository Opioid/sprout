#pragma once

#include "math/vector.hpp"

namespace spectrum {

float3 blackbody(float temperature);

// defined for [1667, 25000] Kelvin
float3 blackbody_fast(float temperature);

}
