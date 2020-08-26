#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_2D_INL
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_2D_INL

#include "interpolated_function_2d.hpp"
#include "math.hpp"
#include "vector2.inl"

namespace math {

template <typename T>
Interpolated_function_2D<T>::Interpolated_function_2D() = default;

template <typename T>
Interpolated_function_2D<T>::Interpolated_function_2D(Interpolated_function_2D&& other) noexcept
    : range_end_(other.range_end_),
      inverse_interval_(other.inverse_interval_),
      num_samples_(other.num_samples_),
      back_(other.back_),
      samples_(other.samples_) {
    other.samples_ = nullptr;
}

template <typename T>
Interpolated_function_2D<T>::Interpolated_function_2D(float2 range_begin, float2 range_end,
                                                      uint2 num_samples, T const t[]) {
    from_array(range_begin, range_end, num_samples, t);
}

template <typename T>
Interpolated_function_2D<T>::~Interpolated_function_2D() {
    delete[] samples_;
}

template <typename T>
void Interpolated_function_2D<T>::from_array(float2 range_begin, float2 range_end,
                                             uint2 num_samples, T const t[]) {
    uint32_t const area = num_samples[0] * num_samples[1];

    if (num_samples_[0] * num_samples_[1] != area) {
        delete[] samples_;

        samples_ = new T[area];
    }

    num_samples_ = num_samples;

    back_ = num_samples_ - 1u;

    range_end_ = range_end;

    float2 const range = range_end - range_begin;

    float2 const interval = range / float2(num_samples - 1u);

    inverse_interval_ = 1.f / interval;

    for (uint32_t i = 0; i < area; ++i) {
        samples_[i] = t[i];
    }
}

template <typename T>
void Interpolated_function_2D<T>::scale(T s) {
    for (uint32_t i = 0, len = num_samples_[0] * num_samples_[1]; i < len; ++i) {
        samples_[i] *= s;
    }
}

template <typename T>
T Interpolated_function_2D<T>::operator()(float x, float y) const {
    x = std::min(x, range_end_[0]);
    y = std::min(y, range_end_[1]);

    float2 const o = float2(x, y) * inverse_interval_;

    uint2 const offset = uint2(o);

    float2 const t = o - float2(offset);

    uint32_t const col1 = std::min(offset[0] + 1, back_[0]);

    uint32_t const row0 = offset[1] * num_samples_[0];
    uint32_t const row1 = std::min(offset[1] + 1, back_[1]) * num_samples_[0];

    T const c00 = samples_[offset[0] + row0];
    T const c10 = samples_[col1 + row0];
    T const c01 = samples_[offset[0] + row1];
    T const c11 = samples_[col1 + row1];

    return bilinear(c00, c10, c01, c11, t[0], t[1]);
}

template <typename T, uint32_t X, uint32_t Y>
Interpolated_function_2D_N<T, X, Y>::Interpolated_function_2D_N() = default;

template <typename T, uint32_t X, uint32_t Y>
Interpolated_function_2D_N<T, X, Y>::Interpolated_function_2D_N(T const t[]) {
    from_array(t);
}

template <typename T, uint32_t X, uint32_t Y>
void Interpolated_function_2D_N<T, X, Y>::from_array(T const t[]) {
    for (uint32_t i = 0; i < X * Y; ++i) {
        samples_[i] = t[i];
    }
}

template <typename T, uint32_t X, uint32_t Y>
T Interpolated_function_2D_N<T, X, Y>::operator()(float x, float y) const {
    x = std::min(x, 1.f);
    y = std::min(y, 1.f);

    float2 const o = float2(x, y) * float2(X - 1, Y - 1);

    uint2 const offset = uint2(o);

    float2 const t = o - float2(offset);

    uint32_t const col1 = std::min(offset[0] + 1, X - 1);

    uint32_t const row0 = offset[1] * X;
    uint32_t const row1 = std::min(offset[1] + 1, Y - 1) * X;

    T const c00 = samples_[offset[0] + row0];
    T const c10 = samples_[col1 + row0];
    T const c01 = samples_[offset[0] + row1];
    T const c11 = samples_[col1 + row1];

    return bilinear(c00, c10, c01, c11, t[0], t[1]);
}

}  // namespace math

#endif
