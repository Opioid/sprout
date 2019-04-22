#ifndef SU_BASE_MATH_VECTOR3_INL
#define SU_BASE_MATH_VECTOR3_INL

#include "exp.hpp"
#include "math.hpp"
#include "simd/simd.inl"
#include "simd_vector.inl"
#include "vector2.inl"
#include "vector3.hpp"

namespace math {

//==============================================================================
// Generic 3D vector
//==============================================================================

template <typename T>
constexpr Vector3<T>::Vector3(T s) noexcept : v{s, s, s} {}

template <typename T>
constexpr Vector3<T>::Vector3(Vector2<T> xy, T z) noexcept : v{xy[0], xy[1], z} {}

template <typename T>
constexpr Vector3<T>::Vector3(T const* a) noexcept : v{a[0], a[1], a[2]} {}

template <typename T>
constexpr Vector3<T>::Vector3(Vector3f_a const& a) noexcept : v{T(a[0]), T(a[1]), T(a[2])} {}

template <typename T>
template <typename U>
constexpr Vector3<T>::Vector3(Vector3<U> const& a) noexcept : v{T(a[0]), T(a[1]), T(a[2])} {}

template <typename T>
constexpr Vector2<T> Vector3<T>::xy() const noexcept {
    return Vector2<T>(v[0], v[1]);
}

template <typename T>
constexpr T Vector3<T>::operator[](uint32_t i) const noexcept {
    return v[i];
}

template <typename T>
constexpr T& Vector3<T>::operator[](uint32_t i) noexcept {
    return v[i];
}

template <typename T>
Vector3<T> Vector3<T>::operator+(T s) const noexcept {
    return Vector3(v[0] + s, v[1] + s, v[2] + s);
}

template <typename T>
Vector3<T> Vector3<T>::operator+(Vector3 const& a) const noexcept {
    return Vector3(v[0] + a[0], v[1] + a[1], v[2] + a[2]);
}

template <typename T>
Vector3<T> Vector3<T>::operator-(T s) const noexcept {
    return Vector3(v[0] - s, v[1] - s, v[2] - s);
}

template <typename T>
Vector3<T> Vector3<T>::operator-(Vector3 const& a) const noexcept {
    return Vector3(v[0] - a[0], v[1] - a[1], v[2] - a[2]);
}

template <typename T>
Vector3<T> Vector3<T>::operator*(Vector3 const& a) const noexcept {
    return Vector3(v[0] * a[0], v[1] * a[1], v[2] * a[2]);
}

template <>
inline Vector3<float> Vector3<float>::operator/(float s) const noexcept {
    float const is = 1.f / s;
    return Vector3<float>(is * v[0], is * v[1], is * v[2]);
}

template <typename T>
Vector3<T> Vector3<T>::operator/(T s) const noexcept {
    return Vector3(v[0] / s, v[1] / s, v[2] / s);
}

template <typename T>
Vector3<T> Vector3<T>::operator/(Vector3 const& a) const noexcept {
    return Vector3(v[0] / a[0], v[1] / a[1], v[2] / a[2]);
}

template <typename T>
constexpr Vector3<T> Vector3<T>::operator<<(uint32_t c) const noexcept {
    return Vector3(v[0] << c, v[1] << c, v[2] << c);
}

template <typename T>
constexpr Vector3<T> Vector3<T>::operator>>(uint32_t c) const noexcept {
    return Vector3(v[0] >> c, v[1] >> c, v[2] >> c);
}

template <typename T>
Vector3<T> Vector3<T>::operator-() const noexcept {
    return Vector3(-v[0], -v[1], -v[2]);
}

template <typename T>
Vector3<T>& Vector3<T>::operator+=(Vector3 const& a) noexcept {
    v[0] += a[0];
    v[1] += a[1];
    v[2] += a[2];
    return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator-=(Vector3 const& a) noexcept {
    v[0] -= a[0];
    v[1] -= a[1];
    v[2] -= a[2];
    return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator*=(Vector3 const& a) noexcept {
    v[0] *= a[0];
    v[1] *= a[1];
    v[2] *= a[2];
    return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator*=(T s) noexcept {
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
    return *this;
}

template <typename T>
Vector3<T>& Vector3<T>::operator/=(T s) noexcept {
    T is = T(1) / s;
    v[0] *= is;
    v[1] *= is;
    v[2] *= is;
    return *this;
}

template <typename T>
bool Vector3<T>::operator==(Vector3 const& a) const noexcept {
    return v[0] == a[0] && v[1] == a[1] && v[2] == a[2];
}

template <typename T>
bool Vector3<T>::operator!=(Vector3 const& a) const noexcept {
    return v[0] != a[0] || v[1] != a[1] || v[2] != a[2];
}

template <typename T>
T Vector3<T>::absolute_max(uint32_t& i) const noexcept {
    T ax = std::abs(v[0]);
    T ay = std::abs(v[1]);
    T az = std::abs(v[2]);

    if (ax >= ay && ax >= az) {
        i = 0;
        return ax;
    }

    if (ay >= ax && ay >= az) {
        i = 1;
        return ay;
    }

    i = 2;
    return az;
}

template <typename T>
static Vector3<T> operator*(T s, Vector3<T> const& v) noexcept {
    return Vector3<T>(s * v[0], s * v[1], s * v[2]);
}

template <typename T>
static T dot(Vector3<T> const& a, Vector3<T> const& b) noexcept {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

template <typename T>
static T length(Vector3<T> const& v) noexcept {
    return std::sqrt(dot(v, v));
}

template <typename T>
static T squared_length(Vector3<T> const& v) noexcept {
    return dot(v, v);
}

template <typename T>
static Vector3<T> normalize(Vector3<T> const& v) noexcept {
    return v / length(v);
}

template <typename T>
static Vector3<T> reciprocal(Vector3<T> const& v) noexcept {
    return Vector3<T>(T(1) / v[0], T(1) / v[1], T(1) / v[2]);
}

template <typename T>
static Vector3<T> cross(Vector3<T> const& a, Vector3<T> const& b) noexcept {
    return Vector3<T>(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
                      a[0] * b[1] - a[1] * b[0]);
}

template <typename T>
static Vector3<T> project(Vector3<T> const& a, Vector3<T> const& b) noexcept {
    return dot(b, a) * b;
}

template <typename T>
static T distance(Vector3<T> const& a, Vector3<T> const& b) noexcept {
    return length(a - b);
}

template <typename T>
static T squared_distance(Vector3<T> const& a, Vector3<T> const& b) noexcept {
    return squared_length(a - b);
}

template <typename T>
Vector3<T> saturate(Vector3<T> const& v) noexcept {
    return Vector3<T>(std::min(std::max(v[0], T(0)), T(1)), std::min(std::max(v[1], T(0)), T(1)),
                      std::min(std::max(v[2], T(0)), T(1)));
}

template <typename T>
static Vector3<T> exp(Vector3<T> const& v) noexcept {
    return Vector3<T>(std::exp(v[0]), std::exp(v[1]), std::exp(v[2]));
}

template <typename T>
static Vector3<T> lerp(Vector3<T> const& a, Vector3<T> const& b, T t) noexcept {
    T u = T(1) - t;
    return u * a + t * b;
}

template <typename T>
static Vector3<T> reflect(Vector3<T> const& normal, Vector3<T> const& v) noexcept {
    return T(2) * dot(v, normal) * normal - v;
}

template <typename T>
static void orthonormal_basis(Vector3<T> const& n, Vector3<T>& t, Vector3<T>& b) noexcept {
    const T sign = std::copysign(T(1), n[2]);
    const T c    = -T(1) / (sign + n[2]);
    const T d    = n[0] * n[1] * c;
    t            = Vector3<T>(T(1) + sign * n[0] * n[0] * c, sign * d, -sign * n[0]);
    b            = Vector3<T>(d, sign + n[1] * n[1] * c, -n[1]);
}

template <typename T>
static Vector3<T> min(Vector3<T> const& v, T s) noexcept {
    return Vector3<T>(std::min(v[0], s), std::min(v[1], s), std::min(v[2], s));
}

template <typename T>
static Vector3<T> min(Vector3<T> const& a, Vector3<T> const& b) noexcept {
    return Vector3<T>(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]));
}

template <typename T>
static Vector3<T> max(Vector3<T> const& v, T s) noexcept {
    return Vector3<T>(std::max(v[0], s), std::max(v[1], s), std::max(v[2], s));
}

template <typename T>
static Vector3<T> max(Vector3<T> const& a, Vector3<T> const& b) noexcept {
    return Vector3<T>(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]));
}

template <typename T>
static Vector3<T> abs(Vector3<T> const& v) noexcept {
    return Vector3<T>(std::abs(v[0]), std::abs(v[1]), std::abs(v[2]));
}

template <typename T>
static bool any_negative(Vector3<T> const& v) noexcept {
    return v[0] < T(0) || v[1] < T(0) || v[2] < T(0);
}

template <typename T>
static bool any_greater_one(Vector3<T> const& v) noexcept {
    return v[0] > T(1) || v[1] > T(1) || v[2] > T(1);
}

template <typename T>
static constexpr bool any_greater_equal(Vector3<T> const& a, Vector3<T> const& b) noexcept {
    return a[0] >= b[0] || a[1] >= b[1] || a[2] >= b[2];
}

template <typename T>
static constexpr bool any_less(Vector3<T> const& a, T s) noexcept {
    return a[0] < s || a[1] < s || a[2] < s;
}

template <typename T>
static constexpr bool all_less(Vector3<T> const& a, T s) noexcept {
    return a[0] < s && a[1] < s && a[2] < s;
}

template <typename T>
static constexpr bool any_less(Vector3<T> const& a, Vector3<T> const& b) noexcept {
    return a[0] < b[0] || a[1] < b[1] || a[2] < b[2];
}

template <typename T>
static bool any_nan(Vector3<T> const& v) noexcept {
    return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]);
}

template <typename T>
static bool any_inf(Vector3<T> const& v) noexcept {
    return std::isinf(v[0]) || std::isinf(v[1]) || std::isinf(v[2]);
}

//==============================================================================
// Aligned 3D float vector
//==============================================================================

inline constexpr Vector3f_a::Vector3f_a(float const* a) noexcept : v{a[0], a[1], a[2], 0.f} {}

inline constexpr Vector3f_a::Vector3f_a(float s) noexcept : v{s, s, s, 0.f} {}

inline constexpr Vector3f_a::Vector3f_a(Vector2<float> const xy, float z) noexcept
    : v{xy[0], xy[1], z, 0.f} {}

template <typename T>
constexpr Vector3f_a::Vector3f_a(Vector3<T> const& a) noexcept
    : v{float(a[0]), float(a[1]), float(a[2]), 0.f} {}

inline constexpr Vector2<float> Vector3f_a::xy() const noexcept {
    return Vector2<float>(v[0], v[1]);
}

inline float constexpr Vector3f_a::operator[](uint32_t i) const noexcept {
    return v[i];
}

inline float constexpr& Vector3f_a::operator[](uint32_t i) noexcept {
    return v[i];
}

inline float Vector3f_a::absolute_max(uint32_t& i) const noexcept {
    float const ax = std::abs(v[0]);
    float const ay = std::abs(v[1]);
    float const az = std::abs(v[2]);

    if (ax >= ay && ax >= az) {
        i = 0;
        return ax;
    }

    if (ay >= ax && ay >= az) {
        i = 1;
        return ay;
    }

    i = 2;
    return az;
}

static inline Vector3f_a constexpr operator+(Vector3f_a const& a, float s) noexcept {
    return Vector3f_a(a[0] + s, a[1] + s, a[2] + s);
}

static inline Vector3f_a constexpr operator+(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    //	__m128 ma = simd::load_float3(a.v);
    //	__m128 mb = simd::load_float3(b.v);
    //	__m128 mr = _mm_add_ps(ma, mb);
    //	Vector3f_a r;
    //	store_float4(r.v, mr);
    //	return r;

    return Vector3f_a(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

static inline Vector3f_a constexpr operator-(Vector3f_a const& a, float s) noexcept {
    return Vector3f_a(a[0] - s, a[1] - s, a[2] - s);
}

static inline Vector3f_a constexpr operator-(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return Vector3f_a(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

static inline Vector3f_a constexpr operator*(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return Vector3f_a(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}

static inline Vector3f_a constexpr operator/(Vector3f_a const& a, float s) noexcept {
    float const is = 1.f / s;
    return Vector3f_a(is * a[0], is * a[1], is * a[2]);
}

static inline Vector3f_a constexpr operator/(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return Vector3f_a(a[0] / b[0], a[1] / b[1], a[2] / b[2]);
}

static inline Vector3f_a constexpr operator+(float s, Vector3f_a const& v) noexcept {
    return Vector3f_a(s + v[0], s + v[1], s + v[2]);
}

static inline Vector3f_a constexpr operator-(float s, Vector3f_a const& v) noexcept {
    return Vector3f_a(s - v[0], s - v[1], s - v[2]);
}

static inline Vector3f_a constexpr operator*(float s, Vector3f_a const& v) noexcept {
    return Vector3f_a(s * v[0], s * v[1], s * v[2]);
}

static inline Vector3f_a constexpr operator/(float s, Vector3f_a const& v) noexcept {
    return Vector3f_a(s / v[0], s / v[1], s / v[2]);
}

static inline Vector3f_a constexpr operator-(Vector3f_a const& v) noexcept {
    return Vector3f_a(-v[0], -v[1], -v[2]);
}

static inline Vector3f_a constexpr& operator+=(Vector3f_a& a, Vector3f_a const& b) noexcept {
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
    return a;
}

static inline Vector3f_a constexpr& operator-=(Vector3f_a& a, float b) noexcept {
    a[0] -= b;
    a[1] -= b;
    a[2] -= b;
    return a;
}

static inline Vector3f_a constexpr& operator-=(Vector3f_a& a, Vector3f_a const& b) noexcept {
    a[0] -= b[0];
    a[1] -= b[1];
    a[2] -= b[2];
    return a;
}

static inline Vector3f_a constexpr& operator*=(Vector3f_a& a, Vector3f_a const& b) noexcept {
    a[0] *= b[0];
    a[1] *= b[1];
    a[2] *= b[2];
    return a;
}

static inline Vector3f_a constexpr& operator*=(Vector3f_a& a, float s) noexcept {
    a[0] *= s;
    a[1] *= s;
    a[2] *= s;
    return a;
}

static inline Vector3f_a constexpr& operator/=(Vector3f_a& a, Vector3f_a const& b) noexcept {
    a[0] /= b[0];
    a[1] /= b[1];
    a[2] /= b[2];
    return a;
}

static inline Vector3f_a constexpr& operator/=(Vector3f_a& a, float s) noexcept {
    float const is = 1.f / s;
    a[0] *= is;
    a[1] *= is;
    a[2] *= is;
    return a;
}

static inline float constexpr dot(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline float length(Vector3f_a const& v) {
    return std::sqrt(dot(v, v));
}

static inline float constexpr squared_length(Vector3f_a const& v) noexcept {
    return dot(v, v);
}

static inline Vector3f_a normalize(Vector3f_a const& v) noexcept {
    // This is slowest on both machines
    //	return v / length(v);

    // This seems to give the best performance on clang & AMD
    return rsqrt(dot(v, v)) * v;
}

static inline Vector3f_a reciprocal(Vector3f_a const& v) noexcept {
    //	return Vector3f_a(1.f / v[0], 1.f / v[1], 1.f / v[2]);

    Vector sx = simd::load_float4(v.v);

    Vector rcp = _mm_rcp_ps(sx);
    rcp        = _mm_and_ps(rcp, simd::Mask3);
    Vector mul = _mm_mul_ps(sx, _mm_mul_ps(rcp, rcp));

    Vector3f_a result;
    simd::store_float4(result.v, _mm_sub_ps(_mm_add_ps(rcp, rcp), mul));
    return result;
}

static inline Vector3f_a constexpr cross(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return Vector3f_a(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
                      a[0] * b[1] - a[1] * b[0]);
}

static inline Vector3f_a constexpr project(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return dot(b, a) * b;
}

static inline float distance(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return length(a - b);
}

static inline float constexpr squared_distance(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return squared_length(a - b);
}

static inline Vector3f_a constexpr saturate(Vector3f_a const& v) noexcept {
    return Vector3f_a(std::clamp(v[0], 0.f, 1.f), std::clamp(v[1], 0.f, 1.f),
                      std::clamp(v[2], 0.f, 1.f));
}

static inline Vector3f_a exp(Vector3f_a const& v) {
    //	return Vector3f_a(math::exp(v[0]), math::exp(v[1]), math::exp(v[2]));
    Vector x = simd::load_float4(v.v);
    x        = exp(x);

    Vector3f_a r;
    simd::store_float4(r.v, x);
    return r;
}

static inline Vector3f_a pow(Vector3f_a const& v, float e) noexcept {
    return Vector3f_a(std::pow(v[0], e), std::pow(v[1], e), std::pow(v[2], e));
}

static inline Vector3f_a constexpr lerp(Vector3f_a const& a, Vector3f_a const& b,
                                        float t) noexcept {
    float const u = 1.f - t;
    return u * a + t * b;
}

static inline Vector3f_a constexpr reflect(Vector3f_a const& normal, Vector3f_a const& v) noexcept {
    return 2.f * dot(v, normal) * normal - v;
}

struct Vector3f_a_pair {
    Vector3f_a a;
    Vector3f_a b;
};

static inline Vector3f_a_pair orthonormal_basis(Vector3f_a const& n) noexcept {
    // Building an Orthonormal Basis, Revisited
    // http://jcgt.org/published/0006/01/01/

    //	float const sign = std::copysign(1.f, n[2]);
    float const sign = copysign1(n[2]);
    float const c    = -1.f / (sign + n[2]);
    float const d    = n[0] * n[1] * c;

    return {Vector3f_a(1.f + sign * n[0] * n[0] * c, sign * d, -sign * n[0]),
            Vector3f_a(d, sign + n[1] * n[1] * c, -n[1])};
}

// https://twitter.com/ian_mallett/status/846631289822232577
/*
static inline void orthonormal_basis_sse(Vector3f_a const& n, Vector3f_a& t,
Vector3f_a& b) { Vector const u = simd::load_float3(n.v);

        float const sign = copysign1(n[2]);

        __m128 temp0 = _mm_set_ps1(1.f / (sign + n[2]));

        __m128 temp0_0 = _mm_shuffle_ps(u, u, _MM_SHUFFLE(0, 1, 0, 0));
        temp0 = _mm_mul_ps(temp0, temp0_0);

        __m128 temp0_1 = _mm_shuffle_ps(u, u, _MM_SHUFFLE(0, 1, 1, 0));
        temp0 = _mm_mul_ps(temp0, temp0_1);

        __m128 temp1 = _mm_shuffle_ps(temp0, u, _MM_SHUFFLE(3, 0, 1, 0));
        __m128 temp2 = _mm_set_ps1(sign);
        __m128 temp3 = _mm_set_ps(0.f, 0.f, 0.f, 1.f);
        temp1 = _mm_mul_ps(temp1, temp2);

        simd::store_float4(t.v, _mm_sub_ps(temp3, temp1));

        __m128 temp4 = _mm_set_ps(0.f, 0.f, sign, 0.f);
        __m128 temp5 = _mm_shuffle_ps(temp0, u, _MM_SHUFFLE(3, 1, 2,1));

        simd::store_float4(b.v, _mm_sub_ps(temp4, temp5));
}*/

static inline Vector3f_a tangent(Vector3f_a const& n) noexcept {
    float const sign = std::copysign(1.f, n[2]);
    float const c    = -1.f / (sign + n[2]);
    float const d    = n[0] * n[1] * c;
    return Vector3f_a(1.f + sign * n[0] * n[0] * c, sign * d, -sign * n[0]);
}

static inline Vector3f_a constexpr min(Vector3f_a const& a, float s) noexcept {
    return Vector3f_a(std::min(a[0], s), std::min(a[1], s), std::min(a[2], s));
}

static inline Vector3f_a constexpr min(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return Vector3f_a(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]));
}

static inline Vector3f_a constexpr max(Vector3f_a const& a, float s) noexcept {
    return Vector3f_a(std::max(a[0], s), std::max(a[1], s), std::max(a[2], s));
}

static inline Vector3f_a constexpr clamp(Vector3f_a const& v, float mi, float ma) noexcept {
    return Vector3f_a(std::min(std::max(v[0], mi), ma), std::min(std::max(v[1], mi), ma),
                      std::min(std::max(v[2], mi), ma));
}

static inline Vector3f_a constexpr max(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return Vector3f_a(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]));
}

static inline Vector3f_a constexpr ceil(Vector3f_a const& v) noexcept {
    return Vector3f_a(std::ceil(v[0]), std::ceil(v[1]), std::ceil(v[2]));
}

static inline float constexpr min_component(Vector3f_a const& v) noexcept {
    return std::min(std::min(v[0], v[1]), v[2]);
}

static inline float constexpr max_component(Vector3f_a const& v) noexcept {
    return std::max(std::max(v[0], v[1]), v[2]);
}

static inline uint32_t constexpr index_min_component(Vector3f_a const& v) noexcept {
    if (v[0] < v[1]) {
        return v[0] < v[2] ? 0 : 2;
    }

    return v[1] < v[2] ? 1 : 2;
}

static inline uint32_t constexpr index_max_component(Vector3f_a const& v) noexcept {
    if (v[0] > v[1]) {
        return v[0] > v[2] ? 0 : 2;
    }

    return v[1] > v[2] ? 1 : 2;
}

static inline float constexpr average(Vector3f_a const& c) noexcept {
    return (c[0] + c[1] + c[2]) / 3.f;
}

static inline float constexpr checksum(Vector3f_a const& v) noexcept {
    return v[0] + v[1] + v[2];
}

static inline Vector3f_a abs(Vector3f_a const& v) noexcept {
    return Vector3f_a(std::abs(v[0]), std::abs(v[1]), std::abs(v[2]));
}

static inline Vector3f_a cos(Vector3f_a const& v) noexcept {
    return Vector3f_a(std::cos(v[0]), std::cos(v[1]), std::cos(v[2]));
}

static inline Vector3f_a sqrt(Vector3f_a const& v) noexcept {
    //	return Vector3f_a(std::sqrt(v[0]), std::sqrt(v[1]), std::sqrt(v[2]));

    Vector x = simd::load_float3(v.v);
    x        = sqrt(x);

    Vector3f_a r;
    simd::store_float4(r.v, x);
    return r;
}

static inline Vector3f_a log(Vector3f_a const& v) noexcept {
    //	return Vector3f_a(std::log(v[0]), std::log(v[1]), std::log(v[2]));

    Vector x = simd::load_float4(v.v);
    x        = log(x);

    Vector3f_a r;
    simd::store_float4(r.v, x);
    return r;
}

static inline constexpr bool operator==(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

static inline constexpr bool operator!=(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2];
}

static inline constexpr bool all_equal_zero(Vector3f_a const& v) noexcept {
    return v[0] == 0.f && v[1] == 0.f && v[2] == 0.f;
}

static inline constexpr bool all_less(Vector3f_a const& v, float s) noexcept {
    return v[0] < s && v[1] < s && v[2] < s;
}

static inline constexpr bool all_less_equal(Vector3f_a const& v, float s) noexcept {
    return v[0] <= s && v[1] <= s && v[2] <= s;
}

static inline constexpr bool all_greater_equal(Vector3f_a const& v, float s) noexcept {
    return v[0] >= s && v[1] >= s && v[2] >= s;
}

static inline constexpr bool any_negative(Vector3f_a const& v) noexcept {
    return v[0] < 0.f || v[1] < 0.f || v[2] < 0.f;
}

static inline constexpr bool any_greater_zero(Vector3f_a const& v) noexcept {
    return v[0] > 0.f || v[1] > 0.f || v[2] > 0.f;
}

static inline constexpr bool any_greater(Vector3f_a const& v, float s) noexcept {
    return v[0] > s || v[1] > s || v[2] > s;
}

static inline constexpr bool any_greater_equal(Vector3f_a const& v, float s) noexcept {
    return v[0] >= s || v[1] >= s || v[2] >= s;
}

static inline constexpr bool any_greater_equal(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return a[0] >= b[0] || a[1] >= b[1] || a[2] >= b[2];
}

static inline constexpr bool any_less(Vector3f_a const& v, float s) noexcept {
    return v[0] < s || v[1] < s || v[2] < s;
}

static inline constexpr bool any_less(Vector3f_a const& a, Vector3f_a const& b) noexcept {
    return a[0] < b[0] || a[1] < b[1] || a[2] < b[2];
}

static inline constexpr bool any_less_equal(Vector3f_a const& v, float s) noexcept {
    return v[0] <= s || v[1] <= s || v[2] <= s;
}

static inline bool any_nan(Vector3f_a const& v) noexcept {
    return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]);
}

static inline bool any_inf(Vector3f_a const& v) noexcept {
    return std::isinf(v[0]) || std::isinf(v[1]) || std::isinf(v[2]);
}

static inline bool all_finite(Vector3f_a const& v) noexcept {
    return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]);
}

static inline bool all_finite_and_positive(Vector3f_a const& v) noexcept {
    return std::isfinite(v[0]) && v[0] >= 0.f && std::isfinite(v[1]) && v[1] >= 0.f &&
           std::isfinite(v[2]) && v[2] >= 0.f;
}

}  // namespace math

#endif
