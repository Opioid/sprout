#ifndef SU_EXTENSION_PROCEDURAL_FLUID_VORTON_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_VORTON_HPP

#include "base/math/vector3.hpp"

namespace procedural::fluid {

struct Vorton {
    float3 accumulate_velocity(float3 const& query_position) const noexcept;

    float3 position;
    float3 vorticity;

    float radius;
};

}  // namespace procedural::fluid

#endif
