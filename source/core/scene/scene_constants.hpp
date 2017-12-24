#ifndef SU_CORE_SCENE_CONSTANTS_HPP
#define SU_CORE_SCENE_CONSTANTS_HPP

// #include <limits>

namespace scene {

constexpr float Ray_max_t = 2.e6f;// 1.e32f;//std::numeric_limits<float>::max();
constexpr float Almost_ray_max_t = Ray_max_t - 0.08f;
constexpr float Almost_ray_max_t_minus_epsilon = Ray_max_t - 0.8f;

}

#endif
