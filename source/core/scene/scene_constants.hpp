#ifndef SU_CORE_SCENE_CONSTANTS_HPP
#define SU_CORE_SCENE_CONSTANTS_HPP

#include <cstdint>

namespace scene {

float constexpr Ray_max_t = 2.e6f;

float constexpr Almost_ray_max_t = Ray_max_t - 0.08f;

float constexpr Almost_ray_max_t_minus_epsilon = Ray_max_t - 0.8f;

uint64_t constexpr Units_per_second = 705600000;

uint64_t constexpr Static_time = 0xffffffffffffffff;

uint64_t constexpr time(double dtime) {
    return static_cast<uint64_t>(static_cast<double>(Units_per_second) * dtime);
}

}  // namespace scene

#endif
