#include "emittance.hpp"
#include "base/spectrum/rgb.inl"
#include "base/math/vector3.inl"

namespace light {

void Emittance::set_flux(float3_p color, float value) {
	float luminance = spectrum::watt_to_lumen(color);

	value_ = value * color / (math::Pi * luminance);

	quantity_ = Quantity::Intensity;
}

void Emittance::set_intensity(float3_p intensity) {
	value_ = intensity;

	quantity_ = Quantity::Intensity;
}

void Emittance::set_intensity(float3_p color, float value) {
	float luminance = spectrum::watt_to_lumen(color);

	value_ = value * color / luminance;

	quantity_ = Quantity::Intensity;
}

void Emittance::set_exitance(float3_p color, float value) {
	float luminance = spectrum::watt_to_lumen(color);

	value_ = value * color / (math::Pi * luminance);

	quantity_ = Quantity::Radiance;
}

void Emittance::set_radiance(float3_p radiance) {
	value_ = radiance;

	quantity_ = Quantity::Radiance;
}

void Emittance::set_luminance(float3_p color, float value) {
	float luminance = spectrum::watt_to_lumen(color);

	value_ = value * color / luminance;

	quantity_ = Quantity::Radiance;
}

float3 Emittance::radiance(float area) const {
	if (Quantity::Intensity == quantity_) {
		return value_ / area;
	} else {
		return value_;
	}
}

}
