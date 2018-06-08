#ifndef SU_BASE_MATH_TRANSFORMATION_HPP
#define SU_BASE_MATH_TRANSFORMATION_HPP

#include "quaternion.hpp"
#include "vector3.hpp"

namespace math {

struct Transformation {
    bool operator==(Transformation const& o) const;

    bool operator!=(Transformation const& o) const;

    float3     position;
    float3     scale;
    Quaternion rotation;
};

}  // namespace math

#endif
