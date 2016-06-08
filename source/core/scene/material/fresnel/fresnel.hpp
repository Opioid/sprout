#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material { namespace fresnel {

math::float3 schlick(float wo_dot_h, math::pfloat3 f0);

float schlick_f0(float n0, float n1);

math::float3 conductor(float wo_dot_h, math::pfloat3 eta, math::pfloat3 k);

float dielectric(float n_dot_wi, float n_dot_wo, float eta);

float dielectric_holgerusan(float cos_theta_i, float cos_theta_t, float eta_i, float eta_t);

math::float3 thinfilm(float wo_dot_h, float external_ior, float thinfilm_ior,
					  float internal_ior, float thickness);

class Schlick {

public:

	Schlick(math::pfloat3 f0);

	math::float3 f(float wo_dot_h) const;

private:

	math::float3 f0_;
};

class Thinfilm {

public:

	Thinfilm(float external_ior, float thinfilm_ior,
			 float internal_ior, float thickness);

	math::float3 f(float wo_dot_h) const;

private:

	float external_ior_;
	float thinfilm_ior_;
	float internal_ior_;
	float thickness_;
};

class Conductor {

public:

	Conductor(math::pfloat3 eta, math::pfloat3 k);

	math::float3 f(float wo_dot_h) const;

private:

	math::float3 eta_;
	math::float3 k_;
};

}}}
