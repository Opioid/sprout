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
      max_t(max_t),
      signs{inv_direction[0] < 0.f ? uint8_t(1u) : uint8_t(0u),
            inv_direction[1] < 0.f ? uint8_t(1u) : uint8_t(0u),
            inv_direction[2] < 0.f ? uint8_t(1u) : uint8_t(0u)} {}

inline void ray::set_direction(float3 const& v) {
    direction     = v;
    inv_direction = reciprocal(v);

    signs[0] = inv_direction[0] < 0.f ? 1u : 0u;
    signs[1] = inv_direction[1] < 0.f ? 1u : 0u;
    signs[2] = inv_direction[2] < 0.f ? 1u : 0u;
}

inline float3 ray::point(float t) const {
    return origin + t * direction;
}

inline float ray::length() const {
    return ::length((min_t - max_t) * direction);
}

}  // namespace math

#endif
