#pragma once

#include "base/math/vector3.hpp"

namespace scene::material::fresnel {

class Schlick {

public:

	Schlick(float f0);
	Schlick(f_float3 f0);

	float3 operator()(float wo_dot_h) const;

private:

	float3 const f0_;
};

class Schlick_blending {

public:

	Schlick_blending(f_float3 a, f_float3 b, float f0);

	float3 operator()(float wo_dot_h) const;

private:

	float3 const a_;
	float3 const b_;
	float const f0_;
};

class Thinfilm {

public:

	Thinfilm(float external_ior, float thinfilm_ior,
			 float internal_ior, float thickness);

	float3 operator()(float wo_dot_h) const;

private:

	float const external_ior_;
	float const thinfilm_ior_;
	float const internal_ior_;
	float const thickness_;
};

class Conductor {

public:

	Conductor(f_float3 eta, f_float3 k);

	float3 operator()(float wo_dot_h) const;

private:

	float3 const eta_;
	float3 const k_;
};

class Constant {

public:

	Constant(float f);
	Constant(f_float3 f);

	float3 operator()(float wo_dot_h) const;

private:

	float3 const f_;
};


template<typename T>
class Weighted {

public:

	Weighted(T const& fresnel, float weight);

	float3 operator()(float wo_dot_h) const;

private:

	const T fresnel_;
	float const weight_;
};

using Schlick_weighted   = Weighted<Schlick>;
using Thinfilm_weighted  = Weighted<Thinfilm>;
using Conductor_weighted = Weighted<Conductor>;

}
