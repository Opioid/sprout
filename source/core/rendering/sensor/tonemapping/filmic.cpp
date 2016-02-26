#include "filmic.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

Filmic::Filmic(math::pvec3 linear_white) : linear_white_(linear_white) {}

math::vec3 Filmic::tonemap(math::pvec3 color) const {
	math::vec3 numerator   = tonemap_function(color);
	math::vec3 denominator = tonemap_function(linear_white_);

	return numerator / denominator;
}

math::vec3 Filmic::tonemap_function(math::pvec3 color) {
	/*
	float a = 0.15f;
	float b = 0.50f;
	float c = 0.10f;
	float d = 0.20f;
	float e = 0.02f;
	float f = 0.30f;
	*/

	// Uncharted
	float a = 0.22f;
	float b = 0.30f;
	float c = 0.10f;
	float d = 0.20f;
	float e = 0.01f;
	float f = 0.30f;

	math::vec3 a_color = a * color;

	return ((color * (a_color + c * b) + d * e) / (color * (a_color + b) + d * f)) - e / f;

/*
	// ACES like in https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;

	return (color * (a * color + b)) / (color * (c * color + d) + e);
*/
}

}}}
