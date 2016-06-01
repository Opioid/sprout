#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace light {

// defined for [1667, 25000] Kelvin
math::float3 blackbody(float temperature);

}}
