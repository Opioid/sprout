#include "aces.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

Aces::Aces(float3_p linear_white, float exposure) :
	white_factor_(white_factor(linear_white, tonemap_function(linear_white))),
	exposure_factor_(std::exp2(exposure)) {}

float3 Aces::tonemap(float3_p color) const {
	return white_factor_ * tonemap_function(exposure_factor_ * color);
}

float3 Aces::tonemap_function(float3_p color) {
	// ACES like in https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;

	return (color * (a * color + b)) / (color * (c * color + d) + e);
}

}}}
