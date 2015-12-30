#include "filmic.hpp"
#include "base/math/vector.inl"

namespace rendering { namespace sensor { namespace tonemapping {

Filmic::Filmic(const math::float3& linear_white) : linear_white_(linear_white) {}

math::float3 Filmic::tonemap(const math::float3& color) const {
	math::float3 numerator	 = tonemap_function(color);
	math::float3 denominator = tonemap_function(linear_white_);

	return numerator / denominator;
}

math::float3 Filmic::tonemap_function(const math::float3& color) {
	/*
	float A = 0.15f;
	float B = 0.50f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.02f;
	float F = 0.30f;
	*/

	float A = 0.22f;
	float B = 0.30f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.01f;
	float F = 0.30f;

	math::float3 A_color = A * color;

	return ((color * (A_color + C * B) + D * E) / (color * (A_color + B) + D * F)) - E / F;
}

}}}
