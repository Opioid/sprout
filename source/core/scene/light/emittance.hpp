#ifndef SU_CORE_SCENE_LIGHT_EMITTANCE_HPP
#define SU_CORE_SCENE_LIGHT_EMITTANCE_HPP

#include "base/math/vector3.hpp"

namespace light {

class Emittance {
  public:
    // Radiometric quantities
    enum class Quantity { Undefined, Flux, Intensity, Radiosty, Radiance };

    // unit: lumen
    void set_luminous_flux(float3_p color, float value);

    // unit: lumen per unit solid angle (lm / sr == candela (cd))
    void set_luminous_intensity(float3_p color, float value);

    // unit: lumen per unit projected area (lumen / m^2)
    void set_luminous_exitance(float3_p color, float value);

    // unit: lumen per unit solid angle per unit projected area (lm / sr / m^2 == cd / m^2)
    void set_luminance(float3_p color, float value);

    // unit: watt per unit solid angle (W / sr)
    void set_intensity(float3_p intensity);

    // unit: watt per unit solid angle per unit projected area (W / sr / m^2)
    void set_radiance(float3_p radiance);

    float3 radiance(float area) const;

  private:
    float3 value_;

    Quantity quantity_;
};

}  // namespace light

#endif
