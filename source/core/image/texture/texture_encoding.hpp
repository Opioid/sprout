#pragma once

namespace image { namespace texture { namespace encoding {

void init();

float srgb_to_float(unsigned char byte);
float snorm_to_float(unsigned char byte);
float unorm_to_float(unsigned char byte);

}}}
