#ifndef SU_EXTENSION_PROCEDURAL_AURORA_SPECTRUM_HPP
#define SU_EXTENSION_PROCEDURAL_AURORA_SPECTRUM_HPP

#include "base/math/vector3.hpp"

namespace procedural::aurora::spectrum {

void init();

float3 linear_rgb(float height);

}  // namespace procedural::aurora::spectrum

#endif
