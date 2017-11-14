#ifndef SU_CORE_IMAGE_TEXTURE_ENCODING_HPP
#define SU_CORE_IMAGE_TEXTURE_ENCODING_HPP

#include <cstdint>

namespace image::texture::encoding {

void init();

float cached_srgb_to_float(uint8_t byte);
float cached_snorm_to_float(uint8_t byte);
float cached_unorm_to_float(uint8_t byte);

}

#endif
