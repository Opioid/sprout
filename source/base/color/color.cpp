#include "color.hpp"
#include "math/vector.inl"

namespace color {

// convert sRGB linear value to sRGB gamma value
float linear_to_sRGB(float c) {
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
Color3 linear_to_sRGB(const Color3& c) {
	return Color3(linear_to_sRGB(c.x), linear_to_sRGB(c.y), linear_to_sRGB(c.z));
}

// convert sRGB gamma value to sRGB linear value
float sRGB_to_linear(float c) {
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
Color3 sRGB_to_linear(const Color3c& c) {
	return Color3(sRGB_to_linear(float(c.x) / 255.f), sRGB_to_linear(float(c.y) / 255.f), sRGB_to_linear(float(c.z) / 255.f));
}

// convert sRGB gamma color to sRGB linear color
Color3 sRGB_to_linear(const Color3& c) {
	return Color3(sRGB_to_linear(c.x), sRGB_to_linear(c.y), sRGB_to_linear(c.z));
}

// convert linear color to gamma color
Color3 linear_to_gamma(const Color3& c, float gamma) {
	float p = 1.f / gamma;

	return Color3(std::pow(c.x, p), std::pow(c.y, p), std::pow(c.z, p));
}

// convert gamma color to linear color
Color3 gamma_to_linear(const Color3& c, float gamma) {
	return Color3(std::pow(c.x, gamma), std::pow(c.y, gamma), std::pow(c.z, gamma));
}

Color3 to_float(const Color3c& c) {
	return Color3(float(c.x) / 255.f, float(c.y) / 255.f, float(c.z) / 255.f);
}

}
