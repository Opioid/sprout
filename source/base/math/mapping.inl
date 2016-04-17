#pragma once

#include "math.hpp"
#include "vector.inl"

namespace math {

math::float3 disk_to_hemisphere_equidistant(math::float2 disk) {
	float longitude = std::atan2(-disk.y, disk.x) ;

	float r = std::sqrt(disk.x * disk.x + disk.y * disk.y);

	// Equidistant projection
	float colatitude = r * math::Pi_div_2;

	// Equal-area projection
	// float colatitude = /*2.f **/ std::asin(r);

	// Stereographic projection
	// float colatitude = 2.f * std::atan(r);

	// spherical to cartesian
	float sin_col = std::sin(colatitude);
	float cos_col = std::cos(colatitude);
	float sin_lon = std::sin(longitude);
	float cos_lon = std::cos(longitude);

	return math::float3(sin_col * cos_lon, sin_col * sin_lon, cos_col);
}

}
