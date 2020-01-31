#include "emittance.hpp"
#include "base/math/vector3.inl"
#include "base/spectrum/rgb.hpp"

namespace light {

void Emittance::set_luminous_flux(float3 const& color, float value) noexcept {
    float const luminance = spectrum::luminance(color);

    value_ = (value / (Pi * luminance)) * color;

    quantity_ = Quantity::Intensity;
}

void Emittance::set_luminous_intensity(float3 const& color, float value) noexcept {
    float const luminance = spectrum::luminance(color);

    value_ = (value / luminance) * color;

    quantity_ = Quantity::Intensity;
}

void Emittance::set_luminous_exitance(float3 const& color, float value) noexcept {
    float const luminance = spectrum::luminance(color);

    value_ = (value / (Pi * luminance)) * color;

    quantity_ = Quantity::Radiance;
}

void Emittance::set_luminance(float3 const& color, float value) noexcept {
    float const luminance = spectrum::luminance(color);

    value_ = (value / luminance) * color;

    quantity_ = Quantity::Radiance;
}

void Emittance::set_intensity(float3 const& intensity) noexcept {
    value_ = intensity;

    quantity_ = Quantity::Intensity;
}

void Emittance::set_radiance(float3 const& radiance) noexcept {
    value_ = radiance;

    quantity_ = Quantity::Radiance;
}

float3 Emittance::radiance(float area) const noexcept {
    if (Quantity::Intensity == quantity_) {
        return value_ / area;
    }
    return value_;
}

}  // namespace light
