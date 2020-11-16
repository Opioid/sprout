#ifndef SU_BASE_MATH_RAY_INL
#define SU_BASE_MATH_RAY_INL

#include "ray.hpp"
#include "vector3.inl"

namespace math {

inline ray::ray() = default;

inline ray::ray(float3_p origin, float3_p direction, float min_t, float max_t)
    : origin(origin[0], origin[1], origin[2], min_t),
      direction(direction[0], direction[1], direction[2], max_t),
      inv_direction(reciprocal(direction)) {}

inline void ray::set_direction(float3_p v) {
    direction     = float3(v[0], v[1], v[2], direction[3]);
    inv_direction = reciprocal(v);
}

inline float ray::min_t() const {
    return origin[3];
}

inline float& ray::min_t() {
    return origin[3];
}

inline float ray::max_t() const {
    return direction[3];
}

inline float& ray::max_t() {
    return direction[3];
}

inline float3 ray::point(float t) const {
    return origin + t * direction;
}

inline float ray::length() const {
    return ::length((min_t() - max_t()) * direction);
}

}  // namespace math

#endif
