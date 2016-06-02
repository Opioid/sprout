#include "emittance.hpp"
#include "base/spectrum/rgb.inl"
#include "base/math/vector.inl"

namespace light {

void Emittance::set_flux(math::pfloat3 color, float value) {
	float luminance = spectrum::watt_to_lumen(color);

	value_ = value * color / (math::Pi * luminance);

	quantity_ = Quantity::Intensity;
}

void Emittance::set_intensity(math::pfloat3 intensity) {
	value_ = intensity;

	quantity_ = Quantity::Intensity;
}

void Emittance::set_intensity(math::pfloat3 color, float value) {
	float luminance = spectrum::watt_to_lumen(color);

	value_ = value * color / luminance;

	quantity_ = Quantity::Intensity;
}

void Emittance::set_exitance(math::pfloat3 color, float value) {
	float luminance = spectrum::watt_to_lumen(color);

	value_ = value * color / (math::Pi * luminance);

	quantity_ = Quantity::Radiance;
}

void Emittance::set_radiance(math::pfloat3 radiance) {
	value_ = radiance;

	quantity_ = Quantity::Radiance;
}

void Emittance::set_luminance(math::pfloat3 color, float value) {
	float luminance = spectrum::watt_to_lumen(color);

	value_ = value * color / luminance;

	quantity_ = Quantity::Radiance;
}

math::float3 Emittance::radiance(float area) const {
	if (Quantity::Intensity == quantity_) {
		return value_ / area;
	} else {
		return value_;
	}
}

}
