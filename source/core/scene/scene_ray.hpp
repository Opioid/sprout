#ifndef SU_CORE_SCENE_RAY_HPP
#define SU_CORE_SCENE_RAY_HPP

#include "base/math/ray.hpp"

namespace scene {

struct Ray : public ray {
    Ray();

    Ray(float3 const& origin, float3 const& direction, float min_t = 0.f, float max_t = 1.f,
        uint32_t depth = 0, float wavelength = 0.f, uint64_t time = 0);

    uint32_t depth;

    float wavelength;

    uint64_t time;
};

struct Ray_differential {
    float3 origin;
    float3 direction;
    float3 x_origin;
    float3 x_direction;
    float3 y_origin;
    float3 y_direction;
};

}  // namespace scene

#endif
