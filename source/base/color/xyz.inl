#pragma once

#include "xyz.hpp"
#include "math/vector.inl"

namespace color {

inline math::float3 xyz_to_linear(math::pfloat3 xyz) {
	return math::float3(
		3.240479f * xyz.x - 1.537150f * xyz.y - 0.498535f * xyz.z,
	   -0.969256f * xyz.x + 1.875991f * xyz.y + 0.041556f * xyz.z,
		0.055648f * xyz.x - 0.204043f * xyz.y + 1.057311f * xyz.z);
}

}
