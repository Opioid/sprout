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

class Schlick_refract {

public:

	Schlick_refract(float f0, float sqrt_eta);
	Schlick_refract(const float3& f0, float sqrt_eta);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 f0_;
	const float sqrt_eta_;
};

class Schlick_conditional {

public:

	Schlick_conditional(float f0, bool full);
	Schlick_conditional(const float3& f0, bool full);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 f0_;
	const bool full_;
};

class Schlick_refract_conditional {

public:

	Schlick_refract_conditional(float f0, float sqrt_eta, bool full);
	Schlick_refract_conditional(const float3& f0, float sqrt_eta, bool full);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 f0_;
	const float sqrt_eta_;
	const bool full_;
};

class Schlick_weighted {

public:

	Schlick_weighted(float f0, float weight);
	Schlick_weighted(const float3& f0, float weight);

	float3 operator()(float wo_dot_h) const;

private:

	const Schlick schlick_;
	const float weight_;
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

	Conductor(const float3& eta, const float3& k);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 eta_;
	const float3 k_;
};

class Conductor_weighted {

public:

	Conductor_weighted(const float3& eta, const float3& k, float weight);

	float3 operator()(float wo_dot_h) const;

private:

	const Conductor conductor_;
	const float weight_;
};

class Constant {

public:

	Constant(float f);
	Constant(const float3& f);

	float3 operator()(float wo_dot_h) const;

private:

	const float3 f_;
};

}
