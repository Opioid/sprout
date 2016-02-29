#include "tonemapper.hpp"

namespace rendering { namespace sensor { namespace tonemapping {

Tonemapper::~Tonemapper() {}

math::vec3 Tonemapper::white_factor(math::pvec3 linear_white, math::pvec3 tonemapped_white) {
	return math::vec3(
				linear_white.x > 0.f ? 1.f / tonemapped_white.x : 1.f,
				linear_white.y > 0.f ? 1.f / tonemapped_white.y : 1.f,
				linear_white.z > 0.f ? 1.f / tonemapped_white.z : 1.f);
}

}}}
