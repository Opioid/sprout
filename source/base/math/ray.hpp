#ifndef SU_BASE_MATH_RAY_HPP
#define SU_BASE_MATH_RAY_HPP

#include "vector3.hpp"

namespace math {

struct ray {
    ray();

    ray(float3_p origin, float3_p direction, float min_t, float max_t);

    void set_direction(float3_p v);

    float  min_t() const;
    float& min_t();

    float  max_t() const;
    float& max_t();

    float3 point(float t) const;

    float length() const;

    float3 origin;
    float3 direction;
    float3 inv_direction;
};

}  // namespace math

#endif
