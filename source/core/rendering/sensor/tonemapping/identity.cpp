#include "identity.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

math::float3 Identity::tonemap(const math::float3& color) const {
	return color;
}

}}}
