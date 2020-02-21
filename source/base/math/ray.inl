#ifndef SU_BASE_MATH_RAY_INL
#define SU_BASE_MATH_RAY_INL

#include "ray.hpp"
#include "vector3.inl"

namespace math {

inline ray::ray() = default;

inline ray::ray(float3 const& origin, float3 const& direction, float min_t, float max_t)
    : origin(origin),
      direction(direction),
      inv_direction(reciprocal(direction)),
      min_t(min_t),
      max_t(max_t) {}

inline void ray::set_direction(float3 const& v) {
    direction     = v;
    inv_direction = reciprocal(v);
}

inline float3 ray::point(float t) const {
    return origin + t * direction;
}

inline float ray::length() const {
    return ::length((min_t - max_t) * direction);
}

}  // namespace math

#endif
