#include "uncharted.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

Uncharted::Uncharted(math::pfloat3 linear_white, float exposure) :
	white_factor_(white_factor(linear_white, tonemap_function(linear_white))),
	exposure_factor_(std::exp2(exposure)) {}

math::float3 Uncharted::tonemap(math::pfloat3 color) const {
	return white_factor_ * tonemap_function(exposure_factor_ * color);
}

math::float3 Uncharted::tonemap_function(math::pfloat3 color) {
	// Uncharted like in http://filmicgames.com/archives/75
	float a = 0.22f;
	float b = 0.30f;
	float c = 0.10f;
	float d = 0.20f;
	float e = 0.01f;
	float f = 0.30f;

	math::float3 a_color = a * color;

	return ((color * (a_color + c * b) + d * e) / (color * (a_color + b) + d * f)) - e / f;
}

}}}
