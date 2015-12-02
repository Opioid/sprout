#include "texture_encoding.hpp"
#include "base/color/color.inl"

namespace image { namespace texture { namespace encoding {

float srgb_float[256];
float snorm_float[256];
float unorm_float[256];

void init() {
	for (uint32_t i = 0; i < 256; ++i) {
		srgb_float[i] = color::sRGB_to_linear(static_cast<float>(i) / 255.f);
	}

	for (uint32_t i = 0; i < 256; ++i) {
		snorm_float[i] = color::snorm_to_float(static_cast<uint8_t>(i));
	}

	for (uint32_t i = 0; i < 256; ++i) {
		unorm_float[i] = color::unorm_to_float(static_cast<uint8_t>(i));
	}
}

float srgb_to_float(uint8_t byte) {
	return srgb_float[byte];
}

float snorm_to_float(uint8_t byte) {
	return snorm_float[byte];
}

float unorm_to_float(uint8_t byte) {
	return unorm_float[byte];
}

}}}

