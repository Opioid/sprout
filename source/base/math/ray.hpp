#pragma once

#include "vector3.hpp"

namespace math {

struct Ray {
	Ray() = default;
	Ray(const float3& origin, const float3& direction,
		float min_t = 0.f, float max_t = 1.f);

	void set_direction(const float3& v);

	float3 point(float t) const;

	float length() const;

	float3 origin;
	float3 direction;
	float3 inv_direction;
	float min_t;
	float max_t;
	uint8_t signs[3];
};

}
