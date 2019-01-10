#ifndef SU_EXTENSION_PROCEDURAL_FLUID_PARTICLE_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_PARTICLE_HPP

#include "base/math/vector3.hpp"

namespace procedural::fluid {

struct Particle {
    float3 position;

    float3 color;
};

}  // namespace procedural::fluid

#endif
