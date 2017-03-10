#pragma once

#include "base/math/vector3.hpp"

namespace scene { namespace material { namespace fresnel {

class Schlick {

public:

	Schlick(float f0);
	Schlick(float3_p f0);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 f0_;
};

class Schlick_weighted {

public:

	Schlick_weighted(float f0, float weight);
	Schlick_weighted(float3_p f0, float weight);

	float3 operator()(float wo_dot_h) const;

private:

	const Schlick schlick_;
	const float weight_;
};

class Schlick_blending {

public:

	Schlick_blending(float3_p a, float3_p b, float f0);

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

class Thinfilm_weighted {

public:

	Thinfilm_weighted(float external_ior, float thinfilm_ior,
					  float internal_ior, float thickness, float weight);

	float3 operator()(float wo_dot_h) const;

private:

	const Thinfilm thinfilm_;
	const float weight_;
};

class Conductor {

public:

	Conductor(float3_p eta, float3_p k);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 eta_;
	const float3 k_;
};

class Conductor_weighted {

public:

	Conductor_weighted(float3_p eta, float3_p k, float weight);

	float3 operator()(float wo_dot_h) const;

private:

	const Conductor conductor_;
	const float weight_;
};

class Constant {

public:

	Constant(float3_p f);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 f_;
};

}}}
