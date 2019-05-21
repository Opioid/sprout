#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_INL
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_INL

#include "interpolated_function.hpp"
#include "math/math.hpp"
#include "memory/align.hpp"

namespace math {

template <typename T>
Interpolated_function<T>::Interpolated_function() noexcept {}

template <typename T>
template <typename F>
Interpolated_function<T>::Interpolated_function(float range_begin, float range_end,
                                                size_t num_samples, F f) noexcept
    : range_end_(range_end),
      num_samples_(num_samples),
      samples_(memory::allocate_aligned<T>(num_samples)) {
    float const range = range_end - range_begin;

    float const interval = range / static_cast<float>(num_samples - 1);

    inverse_range_ = 1.f / interval;

    float s = range_begin;
    for (size_t i = 0; i < num_samples; ++i, s += interval) {
        samples_[i] = f(s);
    }
}

template <typename T>
Interpolated_function<T>::~Interpolated_function() noexcept {
    memory::free_aligned(samples_);
}

template <typename T>
void Interpolated_function<T>::from_array(float range_begin, float range_end, size_t num_samples,
                                          T const t[]) noexcept {
    if (num_samples_ != num_samples) {
        memory::free_aligned(samples_);

        num_samples_ = num_samples;

        samples_ = memory::allocate_aligned<T>(num_samples);
    }

    range_end_ = range_end;

    float const range = range_end - range_begin;

    float const interval = range / static_cast<float>(num_samples - 1);

    inverse_range_ = 1.f / interval;

    for (size_t i = 0; i < num_samples; ++i) {
        samples_[i] = t[i];
    }
}

template <typename T>
T Interpolated_function<T>::operator()(float x) const noexcept {
    x = std::min(x, range_end_);

    float const o = x * inverse_range_;

    uint32_t const offset = static_cast<uint32_t>(o);

    float const t = o - static_cast<float>(offset);

    return lerp(samples_[offset], samples_[offset + 1], t);
}

}  // namespace math

#endif
