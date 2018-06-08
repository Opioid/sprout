#ifndef SU_CORE_SCENE_LIGHT_SAMPLE_HPP
#define SU_CORE_SCENE_LIGHT_SAMPLE_HPP

#include "base/math/vector3.hpp"
#include "scene/shape/shape_sample.hpp"

namespace scene::light {

struct Sample {
    shape::Sample shape;
    float3        radiance;
};

}  // namespace scene::light

#endif
