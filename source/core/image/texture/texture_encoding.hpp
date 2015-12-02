#pragma once

#include <cstdint>

namespace image { namespace texture { namespace encoding {

void init();

float srgb_to_float(uint8_t byte);
float snorm_to_float(uint8_t byte);
float unorm_to_float(uint8_t byte);

}}}
