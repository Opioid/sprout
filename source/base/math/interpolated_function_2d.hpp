#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_2D_HPP
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_2D_HPP

#include "vector2.hpp"

namespace math {

template <typename T>
class Interpolated_function_2D {
  public:
    Interpolated_function_2D() noexcept;

    Interpolated_function_2D(Interpolated_function_2D&& other) noexcept;

    Interpolated_function_2D(float2 range_begin, float2 range_end, uint2 num_samples,
                             T const t[]) noexcept;

    ~Interpolated_function_2D() noexcept;

    void from_array(float2 range_begin, float2 range_end, uint2 num_samples, T const t[]) noexcept;

    void scale(T s) noexcept;

    T operator()(float x, float y) const noexcept;

  private:
    float2 range_end_;

    float2 inverse_interval_;

    uint2 num_samples_ = uint2(0);
    uint2 back_        = uint2(0);

    T* samples_ = nullptr;
};

}  // namespace math

#endif
