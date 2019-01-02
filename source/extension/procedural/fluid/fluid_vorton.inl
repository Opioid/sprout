#ifndef SU_EXTENSION_PROCEDURAL_FLUID_VORTON_INL
#define SU_EXTENSION_PROCEDURAL_FLUID_VORTON_INL

#include <limits>
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "fluid_vorton.hpp"

namespace procedural::fluid {

static const float sAvoidSingularity = std::pow(std::numeric_limits<float>::min(), 1.f / 3.f);

float3 Vorton::accumulate_velocity(float3 const& query_position) const noexcept {
    float3 const vNeighborToSelf = query_position - position;

    const float radius2     = radius * radius;
    const float dist2       = squared_length(vNeighborToSelf) + sAvoidSingularity;
    const float oneOverDist = 1.f / std::sqrt(dist2);

    // If the reciprocal law is used everywhere then when 2 vortices get close, they tend to
    // jettison. Mitigate this by using a linear law when 2 vortices get close to each other.
    const float distLaw = (dist2 < radius2) ? oneOverDist / radius2 : oneOverDist / dist2;
    return (1.f / (4.f * Pi)) * (8.f * radius2 * radius) * distLaw *
           cross(vorticity, vNeighborToSelf);
}

}  // namespace procedural::fluid

#endif
