#include "bssrdf.hpp"
#include "base/math/vector3.inl"

namespace scene { namespace material {

float3 BSSRDF::optical_depth(float length) const {
	return length * (absorption_ + scattering_);
}

const float3& BSSRDF::scattering() const {
	return scattering_;
}

void BSSRDF::set(const float3& absorption, const float3& scattering) {
	absorption_ = absorption;
	scattering_ = scattering;
}

}}
