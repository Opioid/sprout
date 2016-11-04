#pragma once

#include "typed_image.hpp"

namespace image {

float3 average_and_max_3(const Image_float_4& image, float3& max);

float average_and_max_luminance(const Image_float_4& image, float& max);

}
