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

inline ray::ray(float3 const& origin, float3 const& direction, float min_t, float max_t,
                uint32_t depth)
    : origin(origin),
      direction(direction),
      inv_direction(reciprocal(direction)),
      min_t(min_t),
      max_t(max_t),
      signs{inv_direction[0] < 0.f ? uint8_t(1u) : uint8_t(0u),
            inv_direction[1] < 0.f ? uint8_t(1u) : uint8_t(0u),
            inv_direction[2] < 0.f ? uint8_t(1u) : uint8_t(0u)},
      depth(depth) {}

inline ray::ray(float3 const& origin, float3 const& direction, float3 const& inv_direction,
                float min_t, float max_t, uint8_t sign_x, uint8_t sign_y, uint8_t sign_z)
    : origin(origin),
      direction(direction),
      inv_direction(inv_direction),
      min_t(min_t),
      max_t(max_t),
      signs{sign_x, sign_y, sign_z} {}

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

inline ray ray::normalized() const {
    float const length = ::length(direction);
    return ray(origin, direction / length, length * inv_direction, min_t * length, max_t * length,
               signs[0], signs[1], signs[2]);
}

}  // namespace math

#endif
