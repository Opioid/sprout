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

float3 BSSRDF::extinction_coefficient() const {
	return absorption_coefficient_ + scattering_coefficient_;
}

const float3& BSSRDF::absorption_coefficient() const {
	return absorption_coefficient_;
}

const float3& BSSRDF::scattering_coefficient() const {
	return scattering_coefficient_;
}

bool BSSRDF::is_scattering() const {
	return math::any_greater_zero(scattering_coefficient_);
}

void BSSRDF::set(const float3& absorption_coefficient, const float3& scattering_coefficient,
				 float anisotropy) {
	absorption_coefficient_ = absorption_coefficient;
	scattering_coefficient_ = scattering_coefficient;
	anisotropy_ = anisotropy;
}

}
