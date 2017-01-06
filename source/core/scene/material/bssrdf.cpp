#include "bssrdf.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material {

float3_p BSSRDF::scattering() const {
	return scattering_;
}

void BSSRDF::set(float3_p scattering) {
	scattering_ = scattering;
}

}}

