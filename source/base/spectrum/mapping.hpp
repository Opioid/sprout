#ifndef SU_BASE_SPECTRUM_MAPPING_HPP
#define SU_BASE_SPECTRUM_MAPPING_HPP

#include "math/vector.hpp"

namespace spectrum {

float3 heatmap(float x);

byte3 turbo(float x);

float3 blackbody(float temperature);

}  // namespace spectrum

#endif
