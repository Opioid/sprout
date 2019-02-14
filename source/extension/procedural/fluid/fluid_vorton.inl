#ifndef SU_EXTENSION_PROCEDURAL_FLUID_VORTON_INL
#define SU_EXTENSION_PROCEDURAL_FLUID_VORTON_INL

#include <limits>
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "fluid_vorton.hpp"

namespace procedural::fluid {

static float const Avoid_singularity = std::pow(std::numeric_limits<float>::min(), 1.f / 3.f);

float3 Vorton::accumulate_velocity(float3 const& query_position, float radius) const noexcept {
    float3 const d = query_position - position;

    float const radius2  = radius * radius;
    float const dist2    = squared_length(d) + Avoid_singularity;
    float const inv_dist = 1.f / std::sqrt(dist2);

    // If the reciprocal law is used everywhere then when two vortices get close, they tend to
    // jettison. Mitigate this by using a linear law when two vortices get close to each other.
    float const dist_law = (dist2 < radius2) ? inv_dist / radius2 : inv_dist / dist2;

    return (1.f / (4.f * Pi)) * (8.f * radius2 * radius) * dist_law * cross(vorticity, d);
}

void Vorton::assign_by_velocity(float3 const& query_position, float3 const& velocity,
                                float radius) noexcept {
    float3 const relative = query_position - position;

    float const dist = length(relative);

    vorticity = (4.f * Pi) * dist * cross(relative, velocity) / (8.f * radius * radius * radius);
}

}  // namespace procedural::fluid

#endif
