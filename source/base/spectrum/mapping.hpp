#ifndef SU_BASE_SPECTRUM_MAPPING_HPP
#define SU_BASE_SPECTRUM_MAPPING_HPP

#include "math/vector3.hpp"

namespace spectrum {

float3 heatmap(float x);

float3 blackbody(float temperature);

// defined for [1667, 25000] Kelvin
float3 blackbody_fast(float temperature);

}  // namespace spectrum

#endif
