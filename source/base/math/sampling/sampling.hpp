#ifndef SU_BASE_MATH_SAMPLING_SAMPLING_HPP
#define SU_BASE_MATH_SAMPLING_SAMPLING_HPP

#include "math/math.hpp"
#include "math/sincos.hpp"
#include "math/vector3.inl"
#include "math/matrix3x3.inl"

namespace math {

static inline float2 sample_disk_concentric(float2 uv) {
	float const sx = 2.f * uv[0] - 1.f;
	float const sy = 2.f * uv[1] - 1.f;

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

//	float const sin_theta = std::sin(theta);
//	float const cos_theta = std::cos(theta);
	float sin_theta;
	float cos_theta;
	math::sincos(theta, sin_theta, cos_theta);

	return float2(cos_theta * r, sin_theta * r);
}

static inline float2 sample_triangle_uniform(float2 uv) {
	float const su = std::sqrt(uv[0]);
	return float2(1.f - su, uv[1] * su);
}

static inline float3 sample_hemisphere_uniform(float2 uv) {
	float const z = 1.f - uv[0];
	float const r = std::sqrt(1.f - z * z);
	float const phi = uv[1] * (2.f * Pi);

//	float const sin_phi = std::sin(phi);
//	float const cos_phi = std::cos(phi);
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);

	return float3(cos_phi * r, sin_phi * r, z);
}

static inline float3 sample_oriented_hemisphere_uniform(float2 uv,
														f_float3 x,
														f_float3 y,
														f_float3 z) {
	float const za = 1.f - uv[0];
	float const r = std::sqrt(1.f - za * za);
	float const phi = uv[1] * (2.f * Pi);

//	float const sin_phi = std::sin(phi);
//	float const cos_phi = std::cos(phi);
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);

	return (cos_phi * r) * x + (sin_phi * r) * y + za * z;
}

static inline float3 sample_oriented_hemisphere_uniform(float2 uv, float3x3 const& m) {
	float const za = 1.f - uv[0];
	float const r = std::sqrt(1.f - za * za);
	float const phi = uv[1] * (2.f * Pi);

//	float const sin_phi = std::sin(phi);
//	float const cos_phi = std::cos(phi);
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);

	return (cos_phi * r) * m.r[0] + (sin_phi * r) * m.r[1] + za * m.r[2];
}

static inline float3 sample_hemisphere_cosine(float2 uv) {
	float2 const xy = sample_disk_concentric(uv);
	float const  z  = std::sqrt(std::max(0.f, 1.f - xy[0] * xy[0] - xy[1] * xy[1]));

	return float3(xy[0], xy[1], z);
}

static inline float3 sample_oriented_hemisphere_cosine(float2 uv,
													   f_float3 x,
													   f_float3 y,
													   f_float3 z) {
	float2 const xy = sample_disk_concentric(uv);
	float const  za = std::sqrt(std::max(0.f, 1.f - xy[0] * xy[0] - xy[1] * xy[1]));

	return  xy[0] * x + xy[1] * y + za * z;
}

static inline float3 sample_sphere_uniform(float2 uv) {
	float const z = 1.f - 2.f * uv[0];
	float const r = std::sqrt(std::max(0.f, 1.f - z * z));
	float const phi = uv[1] * (2.f * Pi);

//	float const sin_phi = std::sin(phi);
//	float const cos_phi = std::cos(phi);
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);

	return float3(cos_phi * r, sin_phi * r, z);
}

static inline float3 sphere_direction(float sin_theta, float cos_theta, float phi,
									  f_float3 x, f_float3 y, f_float3 z) {
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);

	return sin_theta * cos_phi * x + sin_theta * sin_phi * y + cos_theta * z;
}

static inline float3 sample_oriented_cone_uniform(float2 uv, float cos_theta_max,
												  f_float3 x,
												  f_float3 y,
												  f_float3 z) {
	float const cos_theta = (1.f - uv[0]) + uv[0] * cos_theta_max;
	float const sin_theta = std::sqrt(1.f - cos_theta * cos_theta);
	float const phi = uv[1] * (2.f * Pi);

//	float sin_phi = std::sin(phi);
//	float cos_phi = std::cos(phi);
	float sin_phi;
	float cos_phi;
	math::sincos(phi, sin_phi, cos_phi);

	return (cos_phi * sin_theta) * x + (sin_phi * sin_theta) * y + cos_theta * z;
}

static inline float cone_pdf_uniform(float cos_theta_max) {
	return 1.f / ((2.f * Pi) * (1.f - cos_theta_max));
}

}

#endif
