#include "bssrdf.hpp"
#include "base/math/vector3.inl"

namespace scene { namespace material {

float3 BSSRDF::optical_depth(float length) const {
	return length * (absorption_ + scattering_);
}

float3_p BSSRDF::scattering() const {
	return scattering_;
}

void BSSRDF::set(float3_p absorption, float3_p scattering) {
	absorption_ = absorption;
	scattering_ = scattering;
}

}}

