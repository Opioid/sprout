#include "emittance.hpp"
#include "base/math/vector3.inl"
#include "base/spectrum/rgb.hpp"

namespace light {

void Emittance::set_luminous_flux(float3_p color, float value) {
    float const luminance = spectrum::luminance(color);

    value_ = (value / (Pi * luminance)) * color;

    quantity_ = Quantity::Intensity;
}

void Emittance::set_luminous_intensity(float3_p color, float value) {
    float const luminance = spectrum::luminance(color);

    value_ = (value / luminance) * color;

    quantity_ = Quantity::Intensity;
}

void Emittance::set_luminous_exitance(float3_p color, float value) {
    float const luminance = spectrum::luminance(color);

    value_ = (value / (Pi * luminance)) * color;

    quantity_ = Quantity::Radiance;
}

void Emittance::set_luminance(float3_p color, float value) {
    float const luminance = spectrum::luminance(color);

    value_ = (value / luminance) * color;

    quantity_ = Quantity::Radiance;
}

void Emittance::set_intensity(float3_p intensity) {
    value_ = intensity;

    quantity_ = Quantity::Intensity;
}

void Emittance::set_radiance(float3_p radiance) {
    value_ = radiance;

    quantity_ = Quantity::Radiance;
}

float3 Emittance::radiance(float area) const {
    if (Quantity::Intensity == quantity_) {
        return value_ / area;
    }

    return value_;
}

}  // namespace light
