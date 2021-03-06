#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_1D_INL
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_1D_INL

#include "interpolated_function_1d.hpp"
#include "math/math.hpp"

namespace math {

template <typename T>
Interpolated_function_1D<T>::Interpolated_function_1D() = default;

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
                                                      uint32_t num_samples, F f)
    : range_end_(range_end),
      num_samples_(num_samples),
      back_(num_samples - 1),
      samples_(new T[num_samples]) {
    float const range = range_end - range_begin;

    float const interval = range / float(num_samples - 1);

    inverse_interval_ = 1.f / interval;

    float s = range_begin;
    for (uint32_t i = 0; i < num_samples; ++i, s += interval) {
        samples_[i] = f(s);
    }
}

template <typename T>
Interpolated_function_1D<T>::~Interpolated_function_1D() {
    delete[] samples_;
}

template <typename T>
void Interpolated_function_1D<T>::allocate(float range_begin, float range_end,
                                           uint32_t num_samples) {
    if (num_samples_ != num_samples) {
        delete[] samples_;

        num_samples_ = num_samples;

        back_ = num_samples - 1;

        samples_ = new T[num_samples];
    }

    range_end_ = range_end;

    float const range = range_end - range_begin;

    float const interval = range / float(num_samples - 1);

    inverse_interval_ = 1.f / interval;
}

template <typename T>
void Interpolated_function_1D<T>::from_array(float range_begin, float range_end,
                                             uint32_t num_samples, T const t[]) {
    allocate(range_begin, range_end, num_samples);

    for (uint32_t i = 0; i < num_samples; ++i) {
        samples_[i] = t[i];
    }
}

template <typename T>
void Interpolated_function_1D<T>::scale(T s) {
    for (uint32_t i = 0, len = num_samples_; i < len; ++i) {
        samples_[i] *= s;
    }
}

template <typename T>
T Interpolated_function_1D<T>::operator()(float x) const {
    x = std::min(x, range_end_);

    float const o = x * inverse_interval_;

    uint32_t const offset = uint32_t(o);

    float const t = o - float(offset);

    return lerp(samples_[offset], samples_[std::min(offset + 1, back_)], t);
}

template <typename T>
template <typename I>
T const& Interpolated_function_1D<T>::operator[](I i) const {
    return samples_[i];
}

template <typename T>
template <typename I>
T& Interpolated_function_1D<T>::operator[](I i) {
    return samples_[i];
}

template <typename T, uint32_t N>
Interpolated_function_1D_N<T, N>::Interpolated_function_1D_N() = default;

// template <typename T, uint32_t N>
// Interpolated_function_1D_N<T, N>::Interpolated_function_1D_N(Interpolated_function_1D_N&& other)
// noexcept
//    : range_end_(other.range_end_),
//      inverse_interval_(other.inverse_interval_),
//      samples_{other.samples_} {
//}

template <typename T, uint32_t N>
template <typename F>
Interpolated_function_1D_N<T, N>::Interpolated_function_1D_N(float range_begin, float range_end,
                                                             F f)
    : range_end_(range_end) {
    float const range = range_end - range_begin;

    float const interval = range / float(N - 1);

    inverse_interval_ = 1.f / interval;

    float s = range_begin;
    for (uint32_t i = 0; i < N; ++i, s += interval) {
        samples_[i] = f(s);
    }
}

template <typename T, uint32_t N>
void Interpolated_function_1D_N<T, N>::allocate(float range_begin, float range_end) {
    range_end_ = range_end;

    float const range = range_end - range_begin;

    float const interval = range / float(N - 1);

    inverse_interval_ = 1.f / interval;
}

template <typename T, uint32_t N>
void Interpolated_function_1D_N<T, N>::from_array(float range_begin, float range_end,
                                                  T const t[N]) {
    allocate(range_begin, range_end);

    for (uint32_t i = 0; i < N; ++i) {
        samples_[i] = t[i];
    }
}

template <typename T, uint32_t N>
void Interpolated_function_1D_N<T, N>::scale(T s) {
    for (uint32_t i = 0; i < N; ++i) {
        samples_[i] *= s;
    }
}

template <typename T, uint32_t N>
T Interpolated_function_1D_N<T, N>::operator()(float x) const {
    x = std::min(x, range_end_);

    float const o = x * inverse_interval_;

    uint32_t const offset = uint32_t(o);

    float const t = o - float(offset);

    return lerp(samples_[offset], samples_[std::min(offset + 1, N - 1)], t);
}

template <typename T, uint32_t N>
template <typename I>
T const& Interpolated_function_1D_N<T, N>::operator[](I i) const {
    return samples_[i];
}

template <typename T, uint32_t N>
template <typename I>
T& Interpolated_function_1D_N<T, N>::operator[](I i) {
    return samples_[i];
}

}  // namespace math

#endif
