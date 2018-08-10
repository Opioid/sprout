#ifndef SU_CORE_RENDERING_SENSOR_CLAMP_HPP
#define SU_CORE_RENDERING_SENSOR_CLAMP_HPP

#include "base/math/vector3.hpp"

namespace rendering::sensor::clamp {

class Identity {
  public:
    float4 clamp(float4 const& color) const noexcept;
};

class Clamp {
  public:
    Clamp(float3 const& max) noexcept;

    float4 clamp(float4 const& color) const noexcept;

  private:
    float3 max_;
};

}  // namespace rendering::sensor::clamp

#endif
