#ifndef SU_BASE_MATH_RAY_HPP
#define SU_BASE_MATH_RAY_HPP

#include "vector3.hpp"

namespace math {

struct ray {
    ray();

    ray(float3 const& origin, float3 const& direction, float min_t = 0.f, float max_t = 1.f);

    void set_direction(float3 const& v);

    float3 point(float t) const;

    float length() const;

    float3 origin;
    float3 direction;
    float3 inv_direction;

    float min_t;
    float max_t;

    uint8_t signs[3];
};

}  // namespace math

#endif
