#pragma once

#include "math/vector3.inl"
#include "math/vector4.inl"

namespace encoding {

static inline float unorm_to_float(uint8_t byte) {
	return static_cast<float>(byte) * (1.f / 255.f);
}

static inline float3 unorm_to_float(byte3 c) {
	return float3(unorm_to_float(c[0]),
				  unorm_to_float(c[1]),
				  unorm_to_float(c[2]));
}

static inline float4 unorm_to_float(byte4 c) {
	return float4(unorm_to_float(c[0]),
				  unorm_to_float(c[1]),
				  unorm_to_float(c[2]),
				  unorm_to_float(c[3]));
}

static inline uint8_t float_to_unorm(float x) {
	return static_cast<uint8_t>(x * 255.f + 0.5f);
}

static inline byte3 float_to_unorm(const float3& c) {
	return byte3(float_to_unorm(c[0]),
				 float_to_unorm(c[1]),
				 float_to_unorm(c[2]));
}

static inline byte4 float_to_unorm(const float4& c) {
	return byte4(float_to_unorm(c[0]),
				 float_to_unorm(c[1]),
				 float_to_unorm(c[2]),
				 float_to_unorm(c[3]));
}

static inline float snorm_to_float(uint8_t byte) {
	return static_cast<float>(byte) * (1.f / 128.f) - 1.f;
}

static inline uint8_t float_to_snorm(float x) {
//	return static_cast<uint8_t>((x + 1.f) * (0.5f * 255.f));
	return static_cast<uint8_t>((x + 1.f) * (x > 0.f ? 127.5f : 128.f));
}

static inline byte3 float_to_snorm(const float3& c) {
	return byte3(float_to_snorm(c[0]),
				 float_to_snorm(c[1]),
				 float_to_snorm(c[2]));
}

static inline float snorm16_to_float(int16_t snorm) {
	return static_cast<float>(snorm) * (1.f / 32767.f);
}

static inline int16_t float_to_snorm16(float x) {
	return static_cast<int16_t>(x * 32767.f);
}

}
