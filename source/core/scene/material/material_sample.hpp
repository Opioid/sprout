#pragma once

#include "bxdf.hpp"
#include "base/math/vector.hpp"

namespace sampler {

class Sampler;

}

namespace scene { namespace material {

struct BxDF_result;

class Sample {
public:

	virtual math::float3 evaluate(const math::float3& wi) const = 0;

	virtual math::float3 emission() const = 0;

	virtual math::float3 attenuation() const = 0;

	virtual void sample_evaluate(sampler::Sampler& sampler, BxDF_result& result) const = 0;

	const math::float3& normal() const {
		return n_;
	}

	const math::float3& geometry_normal() const {
		return geo_n_;
	}

	math::float3 tangent_to_world(const math::float3& v) const {
		return math::float3(
			v.x * t_.x + v.y * b_.x + v.z * n_.x,
			v.x * t_.y + v.y * b_.y + v.z * n_.y,
			v.x * t_.z + v.y * b_.z + v.z * n_.z);
	}

	bool same_hemisphere(const math::float3& v) const {
		return math::dot(geo_n_, v) > 0.f;
	}

	void set_basis(const math::float3& t, const math::float3& b, const math::float3& n, const math::float3& geo_n, const math::float3& wo) {
		t_ = t;
		b_ = b;
		n_ = n;
		geo_n_ = geo_n;
		wo_ = wo;
	}

protected:

	math::float3 t_, b_, n_;
	math::float3 geo_n_;
	math::float3 wo_;
};

}}
