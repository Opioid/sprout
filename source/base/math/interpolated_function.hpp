#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_HPP
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_HPP

#include <cstddef>

namespace math {

template <typename T>
class Interpolated_function {
  public:
    Interpolated_function() noexcept;

    template <typename F>
    Interpolated_function(float range_begin, float range_end, size_t num_samples, F f) noexcept;

    ~Interpolated_function() noexcept;

    void from_array(float range_begin, float range_end, size_t num_samples, T const t[]) noexcept;

    T operator()(float x) const noexcept;

  private:
    float range_end_;

    float inverse_interval_;

    size_t num_samples_ = 0;

    T* samples_ = nullptr;
};

}  // namespace math

#endif
