#pragma once

#include "fresnel.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace fresnel {

inline float schlick(float wo_dot_h, float f0) {
	return f0 + std::pow(1.f - wo_dot_h, 5.f) * (1.f - f0);
}

inline math::float3 schlick(float wo_dot_h, math::pfloat3 f0) {
	math::float3 omf(1.f - f0.x, 1.f - f0.y, 1.f - f0.z);
	return f0 + std::pow(1.f - wo_dot_h, 5.f) * omf;

	// Gaussian approximation
	// return f0 + (std::exp2((-5.55473f * wo_dot_h - 6.98316f) * wo_dot_h)) * omf;
}

inline float schlick_f0(float n0, float n1) {
	float t = (n0 - n1) / (n0 + n1);
	return t * t;
}

inline math::float3 conductor(float wo_dot_h, math::pfloat3 eta, math::pfloat3 k) {
	math::float3 tmp_f = eta * eta + k * k;

	float wo_dot_h2 = wo_dot_h * wo_dot_h;
	math::float3 tmp = wo_dot_h2 * tmp_f;

	math::float3 a = 2.f * wo_dot_h * eta;
	math::float3 r_p = (tmp - a + 1.f)
					 / (tmp + a + 1.f);

	math::float3 r_o = (tmp_f - a + wo_dot_h2)
					 / (tmp_f + a + wo_dot_h2);

	return 0.5f * (r_p + r_o);
}

// Adapted from https://seblagarde.wordpress.com/2013/04/29/memo-on-fresnel-equations/#more-1921
inline float dielectric(float n_dot_wi, float n_dot_wo, float eta) {
	float t1 = eta * n_dot_wo;
	float t2 = eta * n_dot_wi;

	float rs = (n_dot_wi + t1) / (n_dot_wi - t1);
	float rp = (n_dot_wo + t2) / (n_dot_wo - t2);

	return 0.5f * (rs * rs + rp * rp);
}

inline float dielectric_holgerusan(float cos_theta_i, float cos_theta_t, float eta_i, float eta_t) {
	float r_p = (eta_t * cos_theta_i + eta_i * cos_theta_t)
			  / (eta_t * cos_theta_i - eta_i * cos_theta_t);

	float r_o = (eta_i * cos_theta_i + eta_t * cos_theta_t)
			  / (eta_i * cos_theta_i - eta_t * cos_theta_t);

	return 0.5f * (r_p * r_p + r_o * r_o);
}

}}}
