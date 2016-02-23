#include "identity.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

math::vec3 Identity::tonemap(math::pvec3 color) const {
	return color;
}

}}}
