#ifndef SU_BASE_MATH_FRUSTUM_HPP
#define SU_BASE_MATH_FRUSTUM_HPP

#include "matrix.hpp"
#include "plane.hpp"

namespace math {

struct AABB;

class Frustum {
  public:
    Frustum();

    Frustum(Plane_p left, Plane_p right, Plane_p top, Plane_p bottom);

    Frustum(float4x4 const& combo_matrix);

    void set_from_matrix(float4x4 const& combo_matrix);

    bool intersect(float3_p p, float radius) const;

    AABB calculate_aabb() const;

  private:
    Plane planes_[6];
};

}  // namespace math

#endif
