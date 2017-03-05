#pragma once

#include "math.hpp"
#include "vector.inl"

namespace math {

inline float3 disk_to_hemisphere_equidistant(float2 uv) {
	float longitude = std::atan2(-uv[1], uv[0]);

	float r = std::sqrt(uv[0] * uv[0] + uv[1] * uv[1]);

	// Equidistant projection
	float colatitude = r * (math::Pi / 2.f);

	// Equal-area projection
	// float colatitude = /*2.f **/ std::asin(r);

	// Stereographic projection
	// float colatitude = 2.f * std::atan(r);

	// spherical to cartesian
	float sin_col = std::sin(colatitude);
	float cos_col = std::cos(colatitude);
	float sin_lon = std::sin(longitude);
	float cos_lon = std::cos(longitude);

	return float3(sin_col * cos_lon, sin_col * sin_lon, cos_col);
}

inline float2 hemisphere_to_disk_equidistant(float3_p dir) {
	// cartesian to spherical
	float colatitude = std::acos(dir[2]);

	float longitude = std::atan2(-dir[1], dir[0]);

	float r = colatitude * (math::Pi_inv * 2.f);

	float sin_lon = std::sin(longitude);
	float cos_lon = std::cos(longitude);

	return float2(r * cos_lon, r * sin_lon);
}

inline float2 hemisphere_to_disk_paraboloid(float3_p dir) {
	float zoi = 1.f / (dir[2] + 1.f);
	return float2(dir[0] * zoi, dir[1] * -zoi);
}

}
