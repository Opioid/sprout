#pragma once

#include "base/math/vector2.hpp"

namespace sampler {

struct Camera_sample {
    int2   pixel;
    float2 pixel_uv;
    float2 lens_uv;
    float  time;
};

}  // namespace sampler
