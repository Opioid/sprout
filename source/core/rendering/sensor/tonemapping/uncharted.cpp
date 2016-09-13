#include "uncharted.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

Uncharted::Uncharted(float3_p linear_white) :
	white_factor_(white_factor(linear_white, tonemap_function(linear_white))) {}

float3 Uncharted::tonemap(float3_p color) const {
	return white_factor_ * tonemap_function(color);
}

float3 Uncharted::tonemap_function(float3_p color) {
	// Uncharted like in http://filmicgames.com/archives/75
	float a = 0.22f;
	float b = 0.30f;
	float c = 0.10f;
	float d = 0.20f;
	float e = 0.01f;
	float f = 0.30f;

	float3 a_color = a * color;

	return ((color * (a_color + c * b) + d * e) / (color * (a_color + b) + d * f)) - e / f;
}

}}}
