#include "bssrdf.hpp"
#include "base/math/vector3.inl"

namespace scene::material {

BSSRDF::BSSRDF(const float3& absorption_coefficient, const float3& scattering_coefficient,
			   float anisotropy) :
	absorption_coefficient_(absorption_coefficient),
	scattering_coefficient_(scattering_coefficient),
	anisotropy_(anisotropy) {}

float3 BSSRDF::optical_depth(float length) const {
	return length * (absorption_coefficient_ + scattering_coefficient_);
}

const float3& BSSRDF::scattering() const {
	return scattering_coefficient_;
}

void BSSRDF::set(const float3& absorption_coefficient, const float3& scattering_coefficient) {
	absorption_coefficient_ = absorption_coefficient;
	scattering_coefficient_ = scattering_coefficient;
}

}
