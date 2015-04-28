#pragma once

#include "base/math/vector.hpp"

namespace sampler {

class Sampler;

}

namespace scene { namespace material {

class Sample {
public:

	virtual math::float3 evaluate(const math::float3& wi) const = 0;

	struct Result {
		math::float3 reflection;
		math::float3 emission;
		math::float3 wi;
		float        pdf;
	};

	virtual void sample_evaluate(sampler::Sampler& sampler, Result& result) const = 0;

	const math::float3& normal() const {
		return n_;
	}

	math::float3 tangent_to_world(const math::float3& v) const {
		return math::float3(
			v.x * t_.x + v.y * b_.x + v.z * n_.x,
			v.x * t_.y + v.y * b_.y + v.z * n_.y,
			v.x * t_.z + v.y * b_.z + v.z * n_.z);
	}

	bool same_hemisphere(const math::float3& v) const {
		return math::dot(n_, v) >= 0.f;
	}

	void set_basis(const math::float3& t, const math::float3& b, const math::float3& n, const math::float3& wo) {
		t_ = t;
		b_ = b;
		n_ = n;
		wo_ = wo;
	}

protected:

	math::float3 t_, b_, n_;

	math::float3 wo_;
};

}}
