#pragma once

#include "sampling.hpp"
#include "math/vector.inl"
#include "math/math.hpp"

namespace math {

inline float2 sample_disk_concentric(float2 uv) {
	const float sx = 2.f * uv.v[0] - 1.f;
	const float sy = 2.f * uv.v[1] - 1.f;

	if (0.f == sx && 0.f == sy) {
		return float2(0.f);
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

	const float sin_theta = std::sin(theta);
	const float cos_theta = std::cos(theta);

	return float2(cos_theta * r, sin_theta * r);
}

inline float2 sample_triangle_uniform(float2 uv) {
	const float su = std::sqrt(uv.v[0]);
	return float2(1.f - su, uv.v[1] * su);
}

inline float3 sample_hemisphere_uniform(float2 uv) {
	const float z = 1.f - uv.v[0];
	const float r = std::sqrt(1.f - z * z);
	const float phi = uv.v[1] * (2.f * Pi);

	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	return float3(cos_phi * r, sin_phi * r, z);
}

inline float3 sample_oriented_hemisphere_uniform(float2 uv, float3_p x, float3_p y, float3_p z) {
	const float za = 1.f - uv.v[0];
	const float r = std::sqrt(1.f - za * za);
	const float phi = uv.v[1] * (2.f * Pi);

	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	return (cos_phi * r) * x + (sin_phi * r) * y + za * z;
}

inline float3 sample_oriented_hemisphere_uniform(float2 uv, const float3x3& m) {
	const float za = 1.f - uv.v[0];
	const float r = std::sqrt(1.f - za * za);
	const float phi = uv.v[1] * (2.f * Pi);

	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	return (cos_phi * r) * m.v3.x + (sin_phi * r) * m.v3.y + za * m.v3.z;
}

inline float3 sample_oriented_hemisphere_uniform(float2 uv, const float4x4& m) {
	const float za = 1.f - uv.v[0];
	const float r = std::sqrt(1.f - za * za);
	const float phi = uv.v[1] * (2.f * Pi);

	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	return (cos_phi * r) * m.v3.x + (sin_phi * r) * m.v3.y + za * m.v3.z;
}

inline float3 sample_hemisphere_cosine(float2 uv) {
	const float2 xy = sample_disk_concentric(uv);
	const float  z  = std::sqrt(std::max(0.f, 1.f - xy.v[0] * xy.v[0] - xy.v[1] * xy.v[1]));

	return float3(xy.v[0], xy.v[1], z);
}

inline float3 sample_oriented_hemisphere_cosine(float2 uv, float3_p x, float3_p y, float3_p z) {
	const float2 xy = sample_disk_concentric(uv);
	const float  za = std::sqrt(std::max(0.f, 1.f - xy.v[0] * xy.v[0] - xy.v[1] * xy.v[1]));

	return  xy.v[0] * x + xy.v[1] * y + za * z;
}

inline float3 sample_sphere_uniform(float2 uv) {
	const float z = 1.f - 2.f * uv.v[0];
	const float r = std::sqrt(std::max(0.f, 1.f - z * z));
	const float phi = uv.v[1] * (2.f * Pi);

	const float sin_phi = std::sin(phi);
	const float cos_phi = std::cos(phi);

	return float3(cos_phi * r, sin_phi * r, z);
}

inline float3 sample_oriented_cone_uniform(float2 uv, float cos_theta_max,
										   float3_p x, float3_p y, float3_p z) {
	const float cos_theta = (1.f - uv.v[0]) + uv.v[0] * cos_theta_max;
	const float sin_theta = std::sqrt(1.f - cos_theta * cos_theta);
	const float phi = uv.v[1] * (2.f * Pi);

	float sin_phi = std::sin(phi);
	float cos_phi = std::cos(phi);

	return (cos_phi * sin_theta) * x + (sin_phi * sin_theta) * y + cos_theta * z;
}

inline float cone_pdf_uniform(float cos_theta_max) {
	return 1.f / ((2.f * Pi) * (1.f - cos_theta_max));
}

}
