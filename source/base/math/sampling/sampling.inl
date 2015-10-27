#pragma once

#include "sampling.hpp"
#include "math/vector.inl"
#include "math/math.hpp"

namespace math {

inline float2 sample_disk_concentric(float2 uv) {
	float sx = 2.f * uv.x - 1.f;
	float sy = 2.f * uv.y - 1.f;

	if (sx == 0.f && sy == 0.f) {
		return float2(0.f, 0.f);
	}

	float r;
	float theta;

	if (sx >= -sy) {
		if (sx > sy) {
			// handle first region of disk
			r = sx;
			if (sy > 0.f) {
				theta = sy / r;
			} else {
				theta = 8.f + sy / r;
			}
		} else {
			// handle second region of disk
			r = sy;
			theta = 2.f - sx / r;
		}
	} else {
		if (sx <= sy) {
			// handle third region of disk
			r = -sx;
			theta = 4.f - sy / r;
		} else {
			// handle fourth region of disk
			r = -sy;
			theta = 6.f + sx / r;
		}
	}

	theta *= Pi / 4.f;

	float sin_theta = std::sin(theta);
	float cos_theta = std::cos(theta);

	return float2(cos_theta * r, sin_theta * r);
}

inline float2 sample_triangle_uniform(float2 uv) {
	float su = std::sqrt(uv.x);
	return float2(1.f - su, uv.y * su);
}

inline float3 sample_hemisphere_uniform(float2 uv) {
	float z = 1.f - uv.x;
	float r = std::sqrt(1.f - z * z);
	float phi = uv.y * 2.f * Pi;

	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	return float3(cos_phi * r, sin_phi * r, z);
}

inline float3 sample_oriented_hemisphere_uniform(float2 uv, const float3& x, const float3& y, const float3& z) {
	float za = 1.f - uv.x;
	float r = std::sqrt(1.f - za * za);
	float phi = uv.y * 2.f * Pi;

	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	return (cos_phi * r) * x + (sin_phi * r) * y + za * z;
}

inline float3 sample_hemisphere_cosine(float2 uv) {
	float2 xy = sample_disk_concentric(uv);
	float  z  = std::sqrt(std::max(0.f, 1.f - xy.x * xy.x - xy.y * xy.y));

	return float3(xy.x, xy.y, z);
}

inline float3 sample_sphere_uniform(float2 uv) {
	float z = 1.f - 2.f * uv.x;
	float r = std::sqrt(std::max(0.f, 1.f - z * z));
	float phi = uv.y * 2.f * Pi;

	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	return float3(cos_phi * r, sin_phi * r, z);
}

inline float3 sample_oriented_cone_uniform(float2 uv, float cos_theta_max,
										   const float3& x, const float3& y, const float3& z) {
	float cos_theta = (1.f - uv.x) + uv.x * cos_theta_max;
	float sin_theta = std::sqrt(1.f - cos_theta * cos_theta);
	float phi = uv.y * 2.f * Pi;

	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	return (cos_phi * sin_theta) * x + (sin_phi * sin_theta) * y + cos_theta * z;
}

inline float cone_pdf_uniform(float cos_theta_max) {
	return 1.f / (2.f * Pi * (1.f - cos_theta_max));
}

}
