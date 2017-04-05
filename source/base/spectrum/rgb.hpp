#pragma once

#include "math/vector4.inl"

namespace spectrum {

constexpr float CIE_constant = 683.002f;

// convert sRGB linear value to sRGB gamma value
static inline float linear_to_sRGB(float c) {
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
static inline float3 linear_RGB_to_sRGB(const float3& c) {
	return float3(linear_to_sRGB(c[0]),
				  linear_to_sRGB(c[1]),
				  linear_to_sRGB(c[2]));
}

// convert sRGB linear color to sRGB gamma color
static inline float4 linear_RGB_to_sRGB(float4_p c) {
	return float4(linear_to_sRGB(c[0]),
				  linear_to_sRGB(c[1]),
				  linear_to_sRGB(c[2]),
				  c[3]);
}

// convert sRGB gamma value to sRGB linear value
static inline float sRGB_to_linear(float c) {
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
static inline float3 sRGB_to_linear_RGB(byte3 c) {
	return float3(sRGB_to_linear(static_cast<float>(c[0]) / 255.f),
				  sRGB_to_linear(static_cast<float>(c[1]) / 255.f),
				  sRGB_to_linear(static_cast<float>(c[2]) / 255.f));
}

// convert sRGB gamma color to sRGB linear color
static inline float3 sRGB_to_linear_RGB(const float3& c) {
	return float3(sRGB_to_linear(c[0]),
				  sRGB_to_linear(c[1]),
				  sRGB_to_linear(c[1]));
}

static inline float4 sRGB_to_linear_RGB(byte4 c) {
	return float4(sRGB_to_linear(static_cast<float>(c[0]) / 255.f),
				  sRGB_to_linear(static_cast<float>(c[1]) / 255.f),
				  sRGB_to_linear(static_cast<float>(c[2]) / 255.f),
				  static_cast<float>(c[3]) / 255.f);
}

// convert linear color to gamma color
static inline float3 linear_to_gamma(const float3& c, float gamma) {
	float p = 1.f / gamma;

	return float3(std::pow(c[0], p),
				  std::pow(c[1], p),
				  std::pow(c[2], p));
}

// convert gamma color to linear color
static inline float3 gamma_to_linear(const float3& c, float gamma) {
	return float3(std::pow(c[0], gamma),
				  std::pow(c[1], gamma),
				  std::pow(c[2], gamma));
}

static inline float luminance(const float3& c) {
	return 0.212671f * c[0] + 0.715160f * c[1] + 0.072169f * c[2];
}

static inline float watt_to_lumen(const float3& c) {
	return CIE_constant * luminance(c);
}

}
