#ifndef SU_CORE_SCENE_RAY_HPP
#define SU_CORE_SCENE_RAY_HPP

#include "base/math/ray.hpp"

namespace scene {

struct Ray : public math::Ray {
    Ray() noexcept = default;

    Ray(float3 const& origin, float3 const& direction, float min_t = 0.f, float max_t = 1.f,
        uint32_t depth = 0, float time = 0.f, float wavelength = 0.f) noexcept;

    uint64_t time_i;

    float time;
    float wavelength;
};

}  // namespace scene

#endif
