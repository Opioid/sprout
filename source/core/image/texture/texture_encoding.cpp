#include "texture_encoding.hpp"
#include "base/encoding/encoding.inl"
#include "base/spectrum/rgb.hpp"

namespace image::texture::encoding {

float srgb_float[256];
float snorm_float[256];
float unorm_float[256];

void init() {
    for (uint32_t i = 0; i < 256; ++i) {
        srgb_float[i] = spectrum::sRGB_to_linear(static_cast<float>(i) * (1.f / 255.f));
    }

    for (uint32_t i = 0; i < 256; ++i) {
        snorm_float[i] = ::encoding::snorm_to_float(static_cast<uint8_t>(i));
    }

    for (uint32_t i = 0; i < 256; ++i) {
        unorm_float[i] = ::encoding::unorm_to_float(static_cast<uint8_t>(i));
    }
}

float cached_srgb_to_float(uint8_t byte) {
    return srgb_float[byte];
}

float cached_snorm_to_float(uint8_t byte) {
    return snorm_float[byte];
}

float cached_unorm_to_float(uint8_t byte) {
    return unorm_float[byte];
}

}  // namespace image::texture::encoding
