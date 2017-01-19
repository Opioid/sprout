#pragma once

#include <cstdint>

namespace image { namespace texture { namespace encoding {

void init();

float cached_srgb_to_float(uint8_t byte);
float cached_snorm_to_float(uint8_t byte);
float cached_unorm_to_float(uint8_t byte);

}}}
