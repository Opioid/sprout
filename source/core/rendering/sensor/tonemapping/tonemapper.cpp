#include "tonemapper.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

Tonemapper::~Tonemapper() {}

float Tonemapper::normalization_factor(float linear_max, float tonemapped_max) {
	return linear_max > 0.f ? 1.f / tonemapped_max : 1.f;
}

}}}
