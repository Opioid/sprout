#include "tonemapper.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

Tonemapper::~Tonemapper() {}

float3 Tonemapper::white_factor(float3_p linear_white, float3_p tonemapped_white) {
	return float3(
				linear_white.x > 0.f ? 1.f / tonemapped_white.x : 1.f,
				linear_white.y > 0.f ? 1.f / tonemapped_white.y : 1.f,
				linear_white.z > 0.f ? 1.f / tonemapped_white.z : 1.f);
}

}}}
