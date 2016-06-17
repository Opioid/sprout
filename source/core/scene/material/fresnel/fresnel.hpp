#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material { namespace fresnel {

math::float3 schlick(float wo_dot_h, math::pfloat3 f0);

float schlick_f0(float n0, float n1);

math::float3 conductor(float wo_dot_h, math::pfloat3 eta, math::pfloat3 k);

float dielectric(float cos_theta_i, float cos_theta_t, float eta_i, float eta_t);

math::float3 thinfilm(float wo_dot_h, float external_ior, float thinfilm_ior,
					  float internal_ior, float thickness);

class Schlick {

public:

	Schlick(float f0);
	Schlick(math::pfloat3 f0);

	math::float3 operator()(float wo_dot_h) const;

private:

	math::float3 f0_;
};

class Schlick_weighted {

public:

	Schlick_weighted(float f0, float weight);
	Schlick_weighted(math::pfloat3 f0, float weight);

	math::float3 operator()(float wo_dot_h) const;

private:

	Schlick schlick_;
	float weight_;
};

class Thinfilm {

public:

	Thinfilm(float external_ior, float thinfilm_ior,
			 float internal_ior, float thickness);

	math::float3 operator()(float wo_dot_h) const;

private:

	float external_ior_;
	float thinfilm_ior_;
	float internal_ior_;
	float thickness_;
};

class Thinfilm_weighted {

public:

	Thinfilm_weighted(float external_ior, float thinfilm_ior,
					  float internal_ior, float thickness, float weight);

	math::float3 operator()(float wo_dot_h) const;

private:

	Thinfilm thinfilm_;
	float weight_;
};

class Conductor {

public:

	Conductor(math::pfloat3 eta, math::pfloat3 k);

	math::float3 operator()(float wo_dot_h) const;

private:

	math::float3 eta_;
	math::float3 k_;
};

}}}
