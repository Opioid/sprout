#include "uncharted.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

Uncharted::Uncharted(float hdr_max) :
	normalization_factor_(normalization_factor(hdr_max, tonemap_function(hdr_max))) {}

float3 Uncharted::tonemap(float3_p color) const {
	return normalization_factor_ * float3(tonemap_function(color.x),
										  tonemap_function(color.y),
										  tonemap_function(color.z));
}

float Uncharted::tonemap_function(float x) {
	// Uncharted like in http://filmicgames.com/archives/75
	float a = 0.22f;
	float b = 0.30f;
	float c = 0.10f;
	float d = 0.20f;
	float e = 0.01f;
	float f = 0.30f;

	float ax = a * x;

	return ((x * (ax + c * b) + d * e) / (x * (ax + b) + d * f)) - e / f;
}

}}}
