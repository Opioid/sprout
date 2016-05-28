#include "radiometry.hpp"
#include "base/math/vector.inl"

namespace light {

void Radiometry::set_intensity(math::pfloat3 intensity) {
	value_ = intensity;

	quantity_ = Quantity::Intensity;
}

void Radiometry::set_radiance(math::pfloat3 radiance) {
	value_ = radiance;

	quantity_ = Quantity::Radiance;
}

math::float3 Radiometry::radiance(float area) const {
	if (Quantity::Intensity == quantity_) {
		return value_ / area;
	} else {
		return value_;
	}
}

}
