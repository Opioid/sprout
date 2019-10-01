#ifndef SU_BASE_MATH_FRUSTUM_HPP
#define SU_BASE_MATH_FRUSTUM_HPP

#include "matrix4x4.hpp"
#include "plane.hpp"

namespace math {

struct AABB;

class Frustum {
  public:
    Frustum() noexcept;

    Frustum(Plane const& left, Plane const& right, Plane const& top, Plane const& bottom) noexcept;

    Frustum(float4x4 const& combo_matrix) noexcept;

    void set_from_matrix(float4x4 const& combo_matrix) noexcept;

    bool intersect(float3 const& p, float radius) const noexcept;

    AABB calculate_aabb() const noexcept;

  private:
    Plane planes_[6];
};

}  // namespace math

#endif
