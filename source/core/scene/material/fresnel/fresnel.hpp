#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material { namespace fresnel {

float schlick(float wo_dot_h, float f0);

float3 schlick(float wo_dot_h, float3_p f0);

float schlick_f0(float n0, float n1);

float3 conductor(float wo_dot_h, float3_p eta, float3_p k);

float dielectric(float cos_theta_i, float cos_theta_t, float eta_i, float eta_t);

float3 thinfilm(float wo_dot_h, float external_ior, float thinfilm_ior,
				float internal_ior, float thickness);

float3 schlick_blending(float wo_dot_h, float3_p a, float3_p b, float f0);

class Schlick {

public:

	Schlick(float f0);
	Schlick(float3_p f0);

	float3 operator()(float wo_dot_h) const;

private:

	float3 f0_;
};

class Schlick_weighted {

public:

	Schlick_weighted(float f0, float weight);
	Schlick_weighted(float3_p f0, float weight);

	float3 operator()(float wo_dot_h) const;

private:

	Schlick schlick_;
	float weight_;
};

class Schlick_blending {

public:

	Schlick_blending(float3_p a, float3_p b, float f0);

	float3 operator()(float wo_dot_h) const;

private:

	float3 a_;
	float3 b_;
	float f0_;
};

class Thinfilm {

public:

	Thinfilm(float external_ior, float thinfilm_ior,
			 float internal_ior, float thickness);

	float3 operator()(float wo_dot_h) const;

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

	float3 operator()(float wo_dot_h) const;

private:

	Thinfilm thinfilm_;
	float weight_;
};

class Conductor {

public:

	Conductor(float3_p eta, float3_p k);

	float3 operator()(float wo_dot_h) const;

private:

	float3 eta_;
	float3 k_;
};

}}}
