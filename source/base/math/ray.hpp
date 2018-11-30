#ifndef SU_BASE_MATH_RAY_HPP
#define SU_BASE_MATH_RAY_HPP

#include "vector3.hpp"

namespace math {

struct ray {
    ray() noexcept = default;

    ray(float3 const& origin, float3 const& direction, float min_t = 0.f,
        float max_t = 1.f) noexcept;

    ray(float3 const& origin, float3 const& direction, float min_t, float max_t,
        uint32_t depth) noexcept;

  private:
    ray(float3 const& origin, float3 const& direction, float3 const& inv_direction, float min_t,
        float max_t, uint8_t sign_x, uint8_t sign_y, uint8_t sign_z) noexcept;

  public:
    void set_direction(float3 const& v) noexcept;

    float3 point(float t) const noexcept;

    float length() const noexcept;

    ray normalized() const noexcept;

    float3 origin;
    float3 direction;
    float3 inv_direction;

    float min_t;
    float max_t;

    uint8_t signs[3];

    uint32_t depth;
};

}  // namespace math

#endif
