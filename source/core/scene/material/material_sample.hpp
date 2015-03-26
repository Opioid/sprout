#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace material {

class Sample {
public:

	virtual math::float3 evaluate(const math::float3& wi) const = 0;

	math::float3 tangent_to_world(const math::float3& v) {
		return math::float3(
			v.x * t_.x + v.y * b_.x + v.z * n_.x,
			v.x * t_.y + v.y * b_.y + v.z * n_.y,
			v.x * t_.z + v.y * b_.z + v.z * n_.z);
	}

protected:

	math::float3 t_, b_, n_;

	math::float3 wo_;
};

}}
