#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_1D_INL
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_1D_INL

#include "interpolated_function_1d.hpp"
#include "math/math.hpp"
#include "memory/align.hpp"

namespace math {

template <typename T>
Interpolated_function_1D<T>::Interpolated_function_1D() noexcept {}

template <typename T>
Interpolated_function_1D<T>::Interpolated_function_1D(Interpolated_function_1D&& other) noexcept
    : range_end_(other.range_end_),
      inverse_interval_(other.inverse_interval_),
      num_samples_(other.num_samples_),
      back_(other.back_),
      samples_(other.samples_) {
    other.samples_ = nullptr;
}

template <typename T>
template <typename F>
Interpolated_function_1D<T>::Interpolated_function_1D(float range_begin, float range_end,
                                                      uint32_t num_samples, F f) noexcept
    : range_end_(range_end),
      num_samples_(num_samples),
      back_(num_samples - 1),
      samples_(memory::allocate_aligned<T>(num_samples)) {
    float const range = range_end - range_begin;

    float const interval = range / static_cast<float>(num_samples - 1);

    inverse_interval_ = 1.f / interval;

    float s = range_begin;
    for (size_t i = 0; i < num_samples; ++i, s += interval) {
        samples_[i] = f(s);
    }
}

template <typename T>
Interpolated_function_1D<T>::~Interpolated_function_1D() noexcept {
    memory::free_aligned(samples_);
}

template <typename T>
void Interpolated_function_1D<T>::from_array(float range_begin, float range_end,
                                             uint32_t num_samples, T const t[]) noexcept {
    if (num_samples_ != num_samples) {
        memory::free_aligned(samples_);

        num_samples_ = num_samples;

        back_ = num_samples - 1;

        samples_ = memory::allocate_aligned<T>(num_samples);
    }

    range_end_ = range_end;

    float const range = range_end - range_begin;

    float const interval = range / static_cast<float>(num_samples - 1);

    inverse_interval_ = 1.f / interval;

    for (uint32_t i = 0; i < num_samples; ++i) {
        samples_[i] = t[i];
    }
}

template <typename T>
void Interpolated_function_1D<T>::scale(T s) noexcept {
    for (uint32_t i = 0, len = num_samples_; i < len; ++i) {
        samples_[i] *= s;
    }
}

template <typename T>
T Interpolated_function_1D<T>::operator()(float x) const noexcept {
    x = std::min(x, range_end_);

    float const o = x * inverse_interval_;

    uint32_t const offset = uint32_t(o);

    float const t = o - static_cast<float>(offset);

    return lerp(samples_[offset], samples_[std::min(offset + 1, back_)], t);
}

}  // namespace math

#endif
