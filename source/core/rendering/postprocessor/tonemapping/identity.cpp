#include "identity.hpp"
#include "base/math/vector4.inl"

namespace rendering { namespace postprocessor { namespace tonemapping {

float3 Identity::tonemap(float3_p color) const {
	return color;
}

}}}
