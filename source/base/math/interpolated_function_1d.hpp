#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_1D_HPP
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_1D_HPP

#include <cstdint>

namespace math {

template <typename T>
class Interpolated_function_1D {
  public:
    Interpolated_function_1D() noexcept;

    Interpolated_function_1D(Interpolated_function_1D&& other) noexcept;

    template <typename F>
	Interpolated_function_1D(float range_begin, float range_end, uint32_t num_samples, F f) noexcept;

    ~Interpolated_function_1D() noexcept;

	void from_array(float range_begin, float range_end, uint32_t num_samples, T const t[]) noexcept;

    void scale(T s) noexcept;

    T operator()(float x) const noexcept;

  private:
    float range_end_;

    float inverse_interval_;

	uint32_t num_samples_ = 0;

    T* samples_ = nullptr;
};

}  // namespace math

#endif
