#pragma once

#include "typed_image.hpp"

namespace image {

float3 average_and_max_3(const Float_4& image, float3& max);

float average_and_max_luminance(const Float_4& image, float& max);

}
