#pragma once

#include "base/math/vector4.inl"

namespace image { namespace texture { namespace sampler {

static inline float bilinear(float c00, float c01, float c10, float c11, float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _t * (_s * c00 + s * c01) + t * (_s * c10 + s * c11);
}

static inline float bilinear(float4_p c, float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _t * (_s * c[0] + s * c[1]) + t * (_s * c[2] + s * c[3]);
}

static inline float2 bilinear(float2 c00, float2 c01, float2 c10, float2 c11,
							  float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _t * (_s * c00 + s * c01) + t * (_s * c10 + s * c11);
}

static inline float2 bilinear(float2 c[4], float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _t * (_s * c[0] + s * c[1]) + t * (_s * c[2] + s * c[3]);
}

static inline float3 bilinear(const float3& c00, const float3& c01, const float3& c10, const float3& c11,
							  float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _t * (_s * c00 + s * c01) + t * (_s * c10 + s * c11);
}

static inline float3 bilinear(float3 c[4], float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _t * (_s * c[0] + s * c[1]) + t * (_s * c[2] + s * c[3]);
}

static inline float4 bilinear(float4_p c00, float4_p c01, float4_p c10, float4_p c11,
							  float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _t * (_s * c00 + s * c01) + t * (_s * c10 + s * c11);
}

}}}
