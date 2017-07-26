#include "emittance.hpp"
#include "base/math/vector3.inl"
#include "base/spectrum/rgb.hpp"

namespace light {

void Emittance::set_flux(const float3& color, float value) {
	const float luminance = spectrum::watt_to_lumen(color);

	value_ = (value / math::Pi * luminance) * color;

	quantity_ = Quantity::Intensity;
}

void Emittance::set_intensity(const float3& intensity) {
	value_ = intensity;

	quantity_ = Quantity::Intensity;
}

void Emittance::set_intensity(const float3& color, float value) {
	const float luminance = spectrum::watt_to_lumen(color);

	value_ = (value / luminance) * color;

	quantity_ = Quantity::Intensity;
}

void Emittance::set_exitance(const float3& color, float value) {
	const float luminance = spectrum::watt_to_lumen(color);

	value_ = (value / math::Pi * luminance) * color;

	quantity_ = Quantity::Radiance;
}

void Emittance::set_radiance(const float3& radiance) {
	value_ = radiance;

	quantity_ = Quantity::Radiance;
}

void Emittance::set_luminance(const float3& color, float value) {
	const float luminance = spectrum::watt_to_lumen(color);

	value_ = (value / luminance) * color;

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
