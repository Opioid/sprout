#pragma once

#include "math/vector.inl"

namespace encoding {

inline float3 unorm_to_float(byte3 c) {
	return float3(static_cast<float>(c.x) / 255.f,
				  static_cast<float>(c.y) / 255.f,
				  static_cast<float>(c.z) / 255.f);
}

inline float4 unorm_to_float(math::byte4 c) {
	return float4(static_cast<float>(c[0]) / 255.f,
				  static_cast<float>(c[1]) / 255.f,
				  static_cast<float>(c[2]) / 255.f,
				  static_cast<float>(c[3]) / 255.f);
}

inline byte3 float_to_unorm(float3_p c) {
	return byte3(static_cast<uint8_t>(c[0] * 255.f),
				 static_cast<uint8_t>(c[1] * 255.f),
				 static_cast<uint8_t>(c[2] * 255.f));
}

inline byte4 float_to_unorm(float4_p c) {
	return byte4(static_cast<uint8_t>(c[0] * 255.f),
				 static_cast<uint8_t>(c[1] * 255.f),
				 static_cast<uint8_t>(c[2] * 255.f),
				 static_cast<uint8_t>(c[3] * 255.f));
}

inline float unorm_to_float(uint8_t byte) {
	return static_cast<float>(byte) / 255.f;
}

inline float snorm_to_float(uint8_t byte) {
	return static_cast<float>(byte) / 128.f - 1.f;
}

inline uint8_t float_to_unorm(float x) {
	return static_cast<uint8_t>(x * 255.f);
}

inline uint8_t float_to_snorm(float x) {
	return static_cast<uint8_t>((x + 1.f) * 0.5f * 255.f);
}

inline byte3 float_to_snorm(float3_p c) {
	return byte3(static_cast<uint8_t>((c[0] + 1.f) * 0.5f * 255.f),
				 static_cast<uint8_t>((c[1] + 1.f) * 0.5f * 255.f),
				 static_cast<uint8_t>((c[2] + 1.f) * 0.5f * 255.f));
}

inline float snorm16_to_float(int16_t snorm) {
	return static_cast<float>(snorm) / 32767.f;
}

inline int16_t float_to_snorm16(float x) {
	return static_cast<int16_t>(x * 32767.f);
}

}
