#ifndef SU_CORE_RENDERING_SENSOR_CLAMP_HPP
#define SU_CORE_RENDERING_SENSOR_CLAMP_HPP

#include "base/math/vector3.hpp"
#include "base/math/vector4.hpp"

namespace rendering::sensor::clamp {

class Identity {
  public:
    float4 clamp(float4_p color) const;
};

class Clamp {
  public:
    Clamp(float3_p max);

    float4 clamp(float4_p color) const;

  private:
    float3 max_;
};

}  // namespace rendering::sensor::clamp

#endif
