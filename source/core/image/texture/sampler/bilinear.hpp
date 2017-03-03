#pragma once

#include "base/math/vector.inl"

namespace image { namespace texture { namespace sampler {

inline float bilinear(float c00, float c01, float c10, float c11, float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float bilinear(float4_p c, float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _s * (_t * c.x + t * c.y) + s * (_t * c.z + t * c.w);
}

inline float2 bilinear(float2 c00, float2 c01, float2 c10, float2 c11,
					   float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float2 bilinear(float2 c[4], float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _s * (_t * c[0] + t * c[1]) + s * (_t * c[2] + t * c[3]);
}

inline float3 bilinear(float3_p c00, float3_p c01, float3_p c10, float3_p c11,
					   float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

inline float3 bilinear(float3 c[4], float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _s * (_t * c[0] + t * c[1]) + s * (_t * c[2] + t * c[3]);
/*
	using namespace math::simd;

	Vector mmt = load_float(t);
	Vector mms = load_float(s);
	Vector mmot = load_float(1.f - t);
	Vector mmos = load_float(1.f - s);

	Vector mm00 = load_float3_unsafe(c[0]);
	Vector mm01 = load_float3_unsafe(c[1]);
	Vector mm10 = load_float3_unsafe(c[2]);
	Vector mm11 = load_float3_unsafe(c[3]);

	mm01 = mul3(mmt, mm01);
	mm11 = mul3(mmt, mm11);
	mm00 = mul3(mmot, mm00);
	mm10 = mul3(mmot, mm10);

	Vector mma = add3(mm00, mm01);
	Vector mmb = add3(mm10, mm11);

	mma = mul3(mmos, mma);
	mmb = mul3(mms, mmb);

	float3 result;
	store_float3_unsafe(result, add3(mma, mmb));

	return result;
	*/
}

inline float4 bilinear(float4_p c00, float4_p c01, float4_p c10, float4_p c11,
					   float s, float t) {
	const float _s = 1.f - s;
	const float _t = 1.f - t;

	return _s * (_t * c00 + t * c01) + s * (_t * c10 + t * c11);
}

}}}
