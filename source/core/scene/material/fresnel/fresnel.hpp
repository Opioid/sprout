#pragma once

#include "base/math/vector3.hpp"

namespace scene::material::fresnel {

class Schlick {

public:

	Schlick(float f0);
	Schlick(const float3& f0);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 f0_;
};

class Schlick_blending {

public:

	Schlick_blending(const float3& a, const float3& b, float f0);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 a_;
	const float3 b_;
	const float f0_;
};

class Thinfilm {

public:

	Thinfilm(float external_ior, float thinfilm_ior,
			 float internal_ior, float thickness);

	float3 operator()(float wo_dot_h) const;

private:

	const float external_ior_;
	const float thinfilm_ior_;
	const float internal_ior_;
	const float thickness_;
};

class Conductor {

public:

	Conductor(const float3& eta, const float3& k);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 eta_;
	const float3 k_;
};

class Constant {

public:

	Constant(float f);
	Constant(const float3& f);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 f_;
};


template<typename T>
class Weighted {

public:

	Weighted(const T& fresnel, float weight);

	float3 operator()(float wo_dot_h) const;

private:

	const T fresnel_;
	const float weight_;
};

using Schlick_weighted   = Weighted<Schlick>;
using Thinfilm_weighted  = Weighted<Thinfilm>;
using Conductor_weighted = Weighted<Conductor>;

}
