#pragma once

#include "vector.hpp"

namespace math {

float2 sample_disk_concentric(math::float2 uv);

float3 sample_hemisphere_cosine(math::float2 uv);

}
