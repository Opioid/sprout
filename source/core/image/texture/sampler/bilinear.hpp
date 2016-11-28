#pragma once

#include "base/math/vector.inl"

namespace image { namespace texture { namespace sampler {

inline float bilinear(float c00, float c01, float c10, float c11, float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float2 bilinear(float2 c00, float2 c01, float2 c10, float2 c11,
					   float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float3 bilinear(float3_p c00, float3_p c01, float3_p c10, float3_p c11,
					   float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float4 bilinear(float4_p c00, float4_p c01, float4_p c10, float4_p c11,
					   float s, float t) {
	float _s = 1.f - s;
	float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

}}}
