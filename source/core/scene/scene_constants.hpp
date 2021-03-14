#ifndef SU_CORE_SCENE_CONSTANTS_HPP
#define SU_CORE_SCENE_CONSTANTS_HPP

#include <cstdint>
#include <cmath>

namespace scene {

// offset_b(std::numeric_limits<float>::max());
inline float constexpr Ray_max_t = 3.4027715434167032e+38f;

// std::nextafter(Ray_max_t, 0.f);
inline float constexpr Almost_ray_max_t = 3.4027713405926072e+38f;

inline uint64_t constexpr Units_per_second = 705600000;

static inline uint64_t time(double dtime) {
    return uint64_t(std::llrint(double(Units_per_second) * dtime));
}

}  // namespace scene

#endif
