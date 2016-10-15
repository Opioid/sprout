#include "identity.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace postprocessor { namespace tonemapping {

float3 Identity::tonemap(float3_p color) const {
	return color;
}

}}}
