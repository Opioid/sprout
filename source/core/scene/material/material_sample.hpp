#pragma once

#include "base/math/vector.hpp"
#include <algorithm>

namespace sampler {

class Sampler;

}

namespace scene { namespace material {

namespace bxdf { struct Result; }

class Sample {
public:

	virtual math::float3 evaluate(const math::float3& wi, float& pdf) const = 0;

	virtual math::float3 emission() const = 0;

	virtual math::float3 attenuation() const = 0;

	virtual void sample_evaluate(sampler::Sampler& sampler, bxdf::Result& result) const = 0;

	virtual bool is_pure_emissive() const = 0;

	virtual bool is_translucent() const = 0;

	float clamped_n_dot_wo() const {
		return std::max(math::dot(n_, wo_), 0.00001f);
	}

	const math::float3& shading_normal() const {
		return n_;
	}

	const math::float3& geometric_normal() const {
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

	void set_basis(const math::float3& t, const math::float3& b, const math::float3& n,
				   const math::float3& geo_n, const math::float3& wo, bool two_sided = false) {
		t_ = t;
		b_ = b;

		if (two_sided && math::dot(geo_n, wo) < 0.f) {
			n_ = -n;
			geo_n_ = -geo_n;
		} else {
			n_ = n;
			geo_n_ = geo_n;
		}

		wo_ = wo;
	}

	static math::float3 attenuation(const math::float3& color, float distance) {
		return math::float3(
			color.x > 0.f ? 1.f / (color.x * distance) : 0.f,
			color.y > 0.f ? 1.f / (color.y * distance) : 0.f,
			color.z > 0.f ? 1.f / (color.z * distance) : 0.f);
	}

protected:

	math::float3 t_, b_, n_;
	math::float3 geo_n_;
	math::float3 wo_;
};

}}
