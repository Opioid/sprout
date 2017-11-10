#ifndef SU_BASE_MATH_MAPPING_INL
#define SU_BASE_MATH_MAPPING_INL

#include "math.hpp"
#include "math/sincos.hpp"
#include "vector3.inl"

namespace math {

static inline float3 disk_to_hemisphere_equidistant(float2 uv) {
	const float longitude = std::atan2(-uv[1], uv[0]);

	const float r = std::sqrt(uv[0] * uv[0] + uv[1] * uv[1]);

	// Equidistant projection
	const float colatitude = r * (math::Pi / 2.f);

	// Equal-area projection
	// float colatitude = /*2.f **/ std::asin(r);

	// Stereographic projection
	// float colatitude = 2.f * std::atan(r);

	// spherical to cartesian
	float sin_col; //= std::sin(colatitude);
	float cos_col; //= std::cos(colatitude);
	math::sincos(colatitude, sin_col, cos_col);
	float sin_lon; //= std::sin(longitude);
	float cos_lon; //= std::cos(longitude);
	math::sincos(longitude, sin_lon, cos_lon);

	return float3(sin_col * cos_lon, sin_col * sin_lon, cos_col);
}

static inline float2 hemisphere_to_disk_equidistant(const float3& dir) {
	// cartesian to spherical
	const float colatitude = std::acos(dir[2]);

	const float longitude = std::atan2(-dir[1], dir[0]);

	const float r = colatitude * (math::Pi_inv * 2.f);

//	float sin_lon = std::sin(longitude);
//	float cos_lon = std::cos(longitude);
	float sin_lon;
	float cos_lon;
	math::sincos(longitude, sin_lon, cos_lon);

	return float2(r * cos_lon, r * sin_lon);
}

static inline float2 hemisphere_to_disk_paraboloid(const float3& dir) {
	const float zoi = 1.f / (dir[2] + 1.f);
	return float2(dir[0] * zoi, dir[1] * -zoi);
}

}

#endif
