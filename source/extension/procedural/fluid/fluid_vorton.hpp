#ifndef SU_EXTENSION_PROCEDURAL_FLUID_VORTON_HPP
#define SU_EXTENSION_PROCEDURAL_FLUID_VORTON_HPP

#include "base/math/vector3.hpp"

namespace procedural::fluid {

class Vorton {
  public:
    float3 accumulate_velocity(float3 const& query_position) const noexcept;

  private:
    float3 position_;
    float3 vorticity_;

    float radius_;
};

}  // namespace procedural::fluid

#endif
