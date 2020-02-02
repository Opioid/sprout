#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_3D_HPP
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_3D_HPP

#include "vector3.hpp"

namespace math {

template <typename T>
class Interpolated_function_3D {
  public:
    Interpolated_function_3D();

    Interpolated_function_3D(Interpolated_function_3D&& other) noexcept;

    Interpolated_function_3D(float3 const& range_begin, float3 const& range_end,
                             uint3 const& num_samples, T const t[]);

    ~Interpolated_function_3D();

    void from_array(float3 const& range_begin, float3 const& range_end, uint3 const& num_samples,
                    T const t[]);

    void scale(T s);

    T operator()(float x, float y, float z) const;

  private:
    float3 range_end_;

    float3 inverse_interval_;

    uint3 num_samples_ = uint3(0u);
    uint3 back_        = uint3(0u);

    T* samples_ = nullptr;
};

}  // namespace math

#endif
