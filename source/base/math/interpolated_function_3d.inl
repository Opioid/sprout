#ifndef SU_BASE_MATH_INTERPOLATED_FUNCTION_3D_INL
#define SU_BASE_MATH_INTERPOLATED_FUNCTION_3D_INL

#include "interpolated_function_3d.hpp"
#include "math.hpp"
#include "vector3.inl"

namespace math {

template <typename T>
Interpolated_function_3D<T>::Interpolated_function_3D() = default;

template <typename T>
Interpolated_function_3D<T>::Interpolated_function_3D(Interpolated_function_3D&& other) noexcept
    : range_end_(other.range_end_),
      inverse_interval_(other.inverse_interval_),
      num_samples_(other.num_samples_),
      back_(other.back_),
      samples_(other.samples_) {
    other.samples_ = nullptr;
}

template <typename T>
Interpolated_function_3D<T>::Interpolated_function_3D(float3_p range_begin, float3_p range_end,
                                                      uint3_p num_samples, T const t[]) {
    from_array(range_begin, range_end, num_samples, t);
}

template <typename T>
Interpolated_function_3D<T>::~Interpolated_function_3D() {
    delete[] samples_;
}

template <typename T>
void Interpolated_function_3D<T>::from_array(float3_p range_begin, float3_p range_end,
                                             uint3_p num_samples, T const t[]) {
    uint32_t const volume = num_samples[0] * num_samples[1] * num_samples[2];

    if (num_samples_[0] * num_samples_[1] * num_samples_[2] != volume) {
        delete[] samples_;

        samples_ = new T[volume];
    }

    num_samples_ = num_samples;

    back_ = num_samples_ - 1u;

    range_end_ = range_end;

    float3 const range = range_end - range_begin;

    float3 const interval = range / float3(num_samples - 1u);

    inverse_interval_ = 1.f / interval;

    for (uint32_t i = 0; i < volume; ++i) {
        samples_[i] = t[i];
    }
}

template <typename T>
void Interpolated_function_3D<T>::scale(T s) {
    for (uint32_t i = 0, len = num_samples_[0] * num_samples_[1] * num_samples_[2]; i < len; ++i) {
        samples_[i] *= s;
    }
}

template <typename T>
T Interpolated_function_3D<T>::operator()(float x, float y, float z) const {
    x = std::min(x, range_end_[0]);
    y = std::min(y, range_end_[1]);
    z = std::min(y, range_end_[1]);

    float3 const o = float3(x, y, z) * inverse_interval_;

    uint3 const offset = uint3(o);

    float3 const t = o - float3(offset);

    uint32_t const col1 = std::min(offset[0] + 1, back_[0]);

    uint32_t const row0 = offset[1] * num_samples_[0];
    uint32_t const row1 = std::min(offset[1] + 1, back_[1]) * num_samples_[0];

    uint32_t const area   = num_samples_[0] * num_samples_[1];
    uint32_t const slice0 = offset[2] * area;
    uint32_t const slice1 = std::min(offset[2] + 1, back_[2]) * area;

    T const c000 = samples_[offset[0] + row0 + slice0];
    T const c100 = samples_[col1 + row0 + slice0];
    T const c010 = samples_[offset[0] + row1 + slice0];
    T const c110 = samples_[col1 + row1 + slice0];
    T const c001 = samples_[offset[0] + row0 + slice1];
    T const c101 = samples_[col1 + row0 + slice1];
    T const c011 = samples_[offset[0] + row1 + slice1];
    T const c111 = samples_[col1 + row1 + slice1];

    T const c0 = bilinear(c000, c100, c010, c110, t[0], t[1]);
    T const c1 = bilinear(c001, c101, c011, c111, t[0], t[1]);

    return lerp(c0, c1, t[2]);
}

template <typename T, uint32_t X, uint32_t Y, uint32_t Z>
Interpolated_function_3D_N<T, X, Y, Z>::Interpolated_function_3D_N() = default;

template <typename T, uint32_t X, uint32_t Y, uint32_t Z>
Interpolated_function_3D_N<T, X, Y, Z>::Interpolated_function_3D_N(T const t[]) {
    from_array(t);
}

template <typename T, uint32_t X, uint32_t Y, uint32_t Z>
void Interpolated_function_3D_N<T, X, Y, Z>::from_array(T const t[]) {
    for (uint32_t i = 0; i < X * Y * Z; ++i) {
        samples_[i] = t[i];
    }
}

template <typename T, uint32_t X, uint32_t Y, uint32_t Z>
T Interpolated_function_3D_N<T, X, Y, Z>::operator()(float x, float y, float z) const {
    x = std::min(x, 1.f);
    y = std::min(y, 1.f);
    z = std::min(z, 1.f);

    float3 const o = float3(x, y, z) * float3(X - 1, Y - 1, Z - 1);

    uint3 const offset = uint3(o);

    float3 const t = o - float3(offset);

    uint32_t const col1 = std::min(offset[0] + 1, X - 1);

    uint32_t const row0 = offset[1] * X;
    uint32_t const row1 = std::min(offset[1] + 1, Y - 1) * X;

    uint32_t constexpr area = X * Y;
    uint32_t const slice0   = offset[2] * area;
    uint32_t const slice1   = std::min(offset[2] + 1, Z - 1) * area;

    T const c000 = samples_[offset[0] + row0 + slice0];
    T const c100 = samples_[col1 + row0 + slice0];
    T const c010 = samples_[offset[0] + row1 + slice0];
    T const c110 = samples_[col1 + row1 + slice0];
    T const c001 = samples_[offset[0] + row0 + slice1];
    T const c101 = samples_[col1 + row0 + slice1];
    T const c011 = samples_[offset[0] + row1 + slice1];
    T const c111 = samples_[col1 + row1 + slice1];

    T const c0 = bilinear(c000, c100, c010, c110, t[0], t[1]);
    T const c1 = bilinear(c001, c101, c011, c111, t[0], t[1]);

    return lerp(c0, c1, t[2]);
}

}  // namespace math

#endif
