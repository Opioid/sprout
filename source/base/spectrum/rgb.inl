#pragma once

#include "rgb.hpp"
#include "math/vector.inl"

namespace spectrum {

// convert sRGB linear value to sRGB gamma value
inline float linear_to_sRGB(float c) {
	if (c <= 0.f) {
		return 0.f;
	} else if (c < 0.0031308f) {
		return 12.92f * c;
	} else if (c < 1.f) {
		return 1.055f * std::pow(c, 0.41666f) - 0.055f;
	}

	return 1.f;
}

// convert sRGB linear color to sRGB gamma color
inline float3 linear_RGB_to_sRGB(float3_p c) {
	return float3(linear_to_sRGB(c.x),
				  linear_to_sRGB(c.y),
				  linear_to_sRGB(c.z));
}

// convert sRGB linear color to sRGB gamma color
inline float4 linear_RGB_to_sRGB(const float4& c) {
	return float4(linear_to_sRGB(c.x),
				  linear_to_sRGB(c.y),
				  linear_to_sRGB(c.z),
				  c.w);
}

// convert sRGB gamma value to sRGB linear value
inline float sRGB_to_linear(float c) {
	if (c <= 0.f) {
		return 0.f;
	} else if (c < 0.04045f) {
		return c / 12.92f;
	} else if (c < 1.f) {
		return std::pow((c + 0.055f) / 1.055f, 2.4f);
	}

	return 1.f;
}

// convert sRGB gamma color to sRGB linear color
inline float3 sRGB_to_linear_RGB(math::byte3 c) {
	return float3(sRGB_to_linear(static_cast<float>(c.x) / 255.f),
				  sRGB_to_linear(static_cast<float>(c.y) / 255.f),
				  sRGB_to_linear(static_cast<float>(c.z) / 255.f));
}

// convert sRGB gamma color to sRGB linear color
inline float3 sRGB_to_linear_RGB(float3_p c) {
	return float3(sRGB_to_linear(c.x),
				  sRGB_to_linear(c.y),
				  sRGB_to_linear(c.z));
}

inline float4 sRGB_to_linear_RGB(math::byte4 c) {
	return float4(sRGB_to_linear(static_cast<float>(c.x) / 255.f),
				  sRGB_to_linear(static_cast<float>(c.y) / 255.f),
				  sRGB_to_linear(static_cast<float>(c.z) / 255.f),
				  static_cast<float>(c.w) / 255.f);
}

// convert linear color to gamma color
inline float3 linear_to_gamma(float3_p c, float gamma) {
	float p = 1.f / gamma;

	return float3(std::pow(c.x, p),
				  std::pow(c.y, p),
				  std::pow(c.z, p));
}

// convert gamma color to linear color
inline float3 gamma_to_linear(float3_p c, float gamma) {
	return float3(std::pow(c.x, gamma),
				  std::pow(c.y, gamma),
				  std::pow(c.z, gamma));
}

inline float3 unorm_to_float(math::byte3 c) {
	return float3(static_cast<float>(c.x) / 255.f,
				  static_cast<float>(c.y) / 255.f,
				  static_cast<float>(c.z) / 255.f);
}

inline float4 unorm_to_float(math::byte4 c) {
	return float4(static_cast<float>(c.x) / 255.f,
				  static_cast<float>(c.y) / 255.f,
				  static_cast<float>(c.z) / 255.f,
				  static_cast<float>(c.w) / 255.f);
}

inline math::byte3 float_to_unorm(float3_p c) {
	return math::byte3(static_cast<uint8_t>(c.x * 255.f),
					   static_cast<uint8_t>(c.y * 255.f),
					   static_cast<uint8_t>(c.z * 255.f));
}

inline math::byte4 float_to_unorm(const float4& c) {
	return math::byte4(static_cast<uint8_t>(c.x * 255.f),
					   static_cast<uint8_t>(c.y * 255.f),
					   static_cast<uint8_t>(c.z * 255.f),
					   static_cast<uint8_t>(c.w * 255.f));
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
	return math::byte3(static_cast<uint8_t>((c.x + 1.f) * 0.5f * 255.f),
					   static_cast<uint8_t>((c.y + 1.f) * 0.5f * 255.f),
					   static_cast<uint8_t>((c.z + 1.f) * 0.5f * 255.f));
}

inline float luminance(float3_p c) {
	return 0.212671f * c.x + 0.715160f * c.y + 0.072169f * c.z;
}

inline float watt_to_lumen(float3_p c) {
	return CIE_constant * luminance(c);
}

}
