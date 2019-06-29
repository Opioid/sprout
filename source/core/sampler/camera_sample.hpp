#ifndef SU_CORE_SAMPLER_CAMERA_SAMPLE_HPP
#define SU_CORE_SAMPLER_CAMERA_SAMPLE_HPP

#include "base/math/vector2.hpp"

namespace sampler {

struct Camera_sample {
    int2   pixel;
    float2 pixel_uv;
    float2 lens_uv;
    float  time;
};

}  // namespace sampler

#endif
