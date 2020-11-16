#ifndef SU_BASE_MATH_FRUSTUM_HPP
#define SU_BASE_MATH_FRUSTUM_HPP

#include "matrix4x4.hpp"
#include "plane.hpp"

namespace math {

struct AABB;

class Frustum {
  public:
    Frustum();

    Frustum(Plane const& left, Plane const& right, Plane const& top, Plane const& bottom);

    Frustum(float4x4 const& combo_matrix);

    void set_from_matrix(float4x4 const& combo_matrix);

    bool intersect(float3_p p, float radius) const;

    AABB calculate_aabb() const;

  private:
    Plane planes_[6];
};

}  // namespace math

#endif
