#include "emittance.hpp"
#include "base/math/vector.inl"

namespace light {

void Emittance::set_intensity(math::pfloat3 intensity) {
	value_ = intensity;

	quantity_ = Quantity::Intensity;
}

void Emittance::set_intensity(math::pfloat3 color, float value) {

}

void Emittance::set_radiance(math::pfloat3 radiance) {
	value_ = radiance;

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
