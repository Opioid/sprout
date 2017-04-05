#include "identity.hpp"
#include "base/math/vector4.inl"

namespace rendering { namespace postprocessor { namespace tonemapping {

float3 Identity::tonemap(const float3& color) const {
	return color;
}

}}}
