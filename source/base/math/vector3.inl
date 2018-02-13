#ifndef SU_BASE_MATH_VECTOR3_INL
#define SU_BASE_MATH_VECTOR3_INL

#include "vector3.hpp"
#include "vector2.inl"
#include "math.hpp"
#include "exp.hpp"
#include "simd_vector.inl"
#include "simd/simd.inl"

namespace math {

//==============================================================================
// Generic 3D vector
//==============================================================================

template<typename T>
constexpr Vector3<T>::Vector3(T s) : v{s, s, s} {}

template<typename T>
constexpr Vector3<T>::Vector3(Vector2<T> xy, T z) : v{xy[0], xy[1], z} {}

template<typename T>
constexpr Vector3<T>::Vector3(const T* v) : v{v[0], v[1], v[2]} {}

template<typename T>
constexpr Vector3<T>::Vector3(const Vector3f_a& v) : v{T(v[0]), T(v[1]), T(v[2])} {}

template<typename T>
constexpr Vector2<T> Vector3<T>::xy() const {
	return Vector2<T>(v[0], v[1]);
}

template<typename T>
constexpr T Vector3<T>::operator[](uint32_t i) const {
	return v[i];
}

template<typename T>
constexpr T& Vector3<T>::operator[](uint32_t i) {
	return v[i];
}

template<typename T>
Vector3<T> Vector3<T>::operator+(T s) const {
	return Vector3(v[0] + s, v[1] + s, v[2] + s);
}

template<typename T>
Vector3<T> Vector3<T>::operator+(const Vector3& a) const {
	return Vector3(v[0] + a[0], v[1] + a[1], v[2] + a[2]);
}

template<typename T>
Vector3<T> Vector3<T>::operator-(T s) const {
	return Vector3(v[0] - s, v[1] - s, v[2] - s);
}

template<typename T>
Vector3<T> Vector3<T>::operator-(const Vector3& a) const {
	return Vector3(v[0] - a[0], v[1] - a[1], v[2] - a[2]);
}

template<typename T>
Vector3<T> Vector3<T>::operator*(const Vector3& a) const {
	return Vector3(v[0] * a[0], v[1] * a[1], v[2] * a[2]);
}

template<typename T>
Vector3<T> Vector3<T>::operator/(T s) const {
	T is = T(1) / s;
	return Vector3(is * v[0], is * v[1], is * v[2]);
}

template<typename T>
Vector3<T> Vector3<T>::operator/(const Vector3& a) const {
	return Vector3(v[0] / a[0], v[1] / a[1], v[2] / a[2]);
}

template<typename T>
Vector3<T> Vector3<T>::operator-() const {
	return Vector3(-v[0], -v[1], -v[2]);
}

template<typename T>
Vector3<T>& Vector3<T>::operator+=(const Vector3& a) {
	v[0] += a[0];
	v[1] += a[1];
	v[2] += a[2];
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator-=(const Vector3& a) {
	v[0] -= a[0];
	v[1] -= a[1];
	v[2] -= a[2];
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator*=(const Vector3& a) {
	v[0] *= a[0];
	v[1] *= a[1];
	v[2] *= a[2];
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator*=(T s) {
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;
	return *this;
}

template<typename T>
Vector3<T>& Vector3<T>::operator/=(T s) {
	T is = T(1) / s;
	v[0] *= is;
	v[1] *= is;
	v[2] *= is;
	return *this;
}

template<typename T>
bool Vector3<T>::operator==(const Vector3& a) const {
	return v[0] == a[0] && v[1] == a[1] && v[2] == a[2];
}

template<typename T>
bool Vector3<T>::operator!=(const Vector3& a) const {
	return v[0] != a[0] || v[1] != a[1] || v[2] != a[2];
}

template<typename T>
T Vector3<T>::absolute_max(uint32_t& i) const {
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

template<typename T>
const Vector3<T> Vector3<T>::identity(T(0), T(0), T(0));

template<typename T>
static Vector3<T> operator*(T s, const Vector3<T>& v) {
	return Vector3<T>(s * v[0], s * v[1], s * v[2]);
}

template<typename T>
static T dot(const Vector3<T>& a, const Vector3<T>& b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

template<typename T>
static T length(const Vector3<T>& v) {
	return std::sqrt(dot(v, v));
}

template<typename T>
static T squared_length(const Vector3<T>& v) {
	return dot(v, v);
}

template<typename T>
static Vector3<T> normalize(const Vector3<T>& v) {
	return v / length(v);
}

template<typename T>
static Vector3<T> reciprocal(const Vector3<T>& v) {
	return Vector3<T>(T(1) / v[0], T(1) / v[1], T(1) / v[2]);
}

template<typename T>
static Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(a[1] * b[2] - a[2] * b[1],
					  a[2] * b[0] - a[0] * b[2],
					  a[0] * b[1] - a[1] * b[0]);
}

template<typename T>
static Vector3<T> project(const Vector3<T>& a, const Vector3<T>& b) {
	return dot(b, a) * b;
}

template<typename T>
static T distance(const Vector3<T>& a, const Vector3<T>& b) {
	return length(a - b);
}

template<typename T>
static T squared_distance(const Vector3<T>& a, const Vector3<T>& b) {
	return squared_length(a - b);
}

template<typename T>
Vector3<T> saturate(const Vector3<T>& v) {
	return Vector3<T>(std::min(std::max(v[0], T(0)), T(1)),
					  std::min(std::max(v[1], T(0)), T(1)),
					  std::min(std::max(v[2], T(0)), T(1)));
}

template<typename T>
static Vector3<T> exp(const Vector3<T>& v) {
	return Vector3<T>(std::exp(v[0]), std::exp(v[1]), std::exp(v[2]));
}

template<typename T>
static Vector3<T> lerp(const Vector3<T>& a, const Vector3<T>& b, T t) {
	T u = T(1) - t;
	return u * a + t * b;
}

template<typename T>
static Vector3<T> reflect(const Vector3<T>& normal, const Vector3<T>& v) {
	return T(2) * dot(v, normal) * normal - v;
}

template<typename T>
static void orthonormal_basis(const Vector3<T>& n, Vector3<T>& t, Vector3<T>& b) {
	const T sign = std::copysign(T(1), n[2]);
	const T c = -T(1) / (sign + n[2]);
	const T d = n[0] * n[1] * c;
	t = Vector3<T>(T(1) + sign * n[0] * n[0] * c, sign * d, -sign * n[0]);
	b = Vector3<T>(d, sign + n[1] * n[1] * c, -n[1]);
}

template<typename T>
static Vector3<T> min(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]));
}

template<typename T>
static Vector3<T> max(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]));
}

template<typename T>
static Vector3<T> abs(const Vector3<T>& v) {
	return Vector3<T>(std::abs(v[0]), std::abs(v[1]), std::abs(v[2]));
}

template<typename T>
static bool any_negative(const Vector3<T>& v) {
	return v[0] < T(0) || v[1] < T(0) || v[2] < T(0);
}

template<typename T>
static bool any_greater_one(const Vector3<T>& v) {
	return v[0] > T(1) || v[1] > T(1) || v[2] > T(1);
}

template<typename T>
static bool any_nan(const Vector3<T>& v) {
	return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]);
}

template<typename T>
static bool any_inf(const Vector3<T>& v) {
	return std::isinf(v[0]) || std::isinf(v[1]) || std::isinf(v[2]);
}

//==============================================================================
// Aligned 3D float vector
//==============================================================================

inline constexpr Vector3f_a::Vector3f_a(const float* v) : v{v[0], v[1], v[2], 0.f} {}

inline constexpr Vector3f_a::Vector3f_a(float s) : v{s, s, s, 0.f} {}

inline constexpr Vector3f_a::Vector3f_a(const Vector2<float> xy, float z) :
	v{xy[0], xy[1], z, 0.f} {}

template<typename T>
constexpr Vector3f_a::Vector3f_a(const Vector3<T>& v) :
	v{float(v[0]), float(v[1]), float(v[2]), 0.f} {}

inline constexpr Vector2<float> Vector3f_a::xy() const {
	return Vector2<float>(v[0], v[1]);
}

inline constexpr float Vector3f_a::operator[](uint32_t i) const{
	return v[i];
}

inline constexpr float& Vector3f_a::operator[](uint32_t i) {
	return v[i];
}

inline float Vector3f_a::absolute_max(uint32_t& i) const {
	const float ax = std::abs(v[0]);
	const float ay = std::abs(v[1]);
	const float az = std::abs(v[2]);

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

inline constexpr Vector3f_a Vector3f_a::identity() {
	return Vector3f_a(0.f, 0.f, 0.f);
}

static inline constexpr Vector3f_a operator+(const Vector3f_a& a, float s) {
	return Vector3f_a(a[0] + s, a[1] + s, a[2] + s);
}

static inline constexpr Vector3f_a operator+(const Vector3f_a& a, const Vector3f_a& b) {
//	__m128 ma = simd::load_float3(a.v);
//	__m128 mb = simd::load_float3(b.v);
//	__m128 mr = _mm_add_ps(ma, mb);
//	Vector3f_a r;
//	store_float4(r.v, mr);
//	return r;

	return Vector3f_a(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

static inline constexpr Vector3f_a operator-(const Vector3f_a& a, float s) {
	return Vector3f_a(a[0] - s, a[1] - s, a[2] - s);
}

static inline constexpr Vector3f_a operator-(const Vector3f_a& a, const Vector3f_a& b) {
	return Vector3f_a(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}

static inline constexpr Vector3f_a operator*(const Vector3f_a& a, const Vector3f_a& b) {
	return Vector3f_a(a[0] * b[0], a[1] * b[1], a[2] * b[2]);
}

static inline constexpr Vector3f_a operator/(const Vector3f_a& a, float s) {
	const float is = 1.f / s;
	return Vector3f_a(is * a[0], is * a[1], is * a[2]);
}

static inline constexpr Vector3f_a operator/(const Vector3f_a& a, const Vector3f_a& b) {
	return Vector3f_a(a[0] / b[0], a[1] / b[1], a[2] / b[2]);
}

static inline constexpr Vector3f_a operator+(float s, const Vector3f_a& v) {
	return Vector3f_a(s + v[0], s + v[1], s + v[2]);
}

static inline constexpr Vector3f_a operator-(float s, const Vector3f_a& v) {
	return Vector3f_a(s - v[0], s - v[1], s - v[2]);
}

static inline constexpr Vector3f_a operator*(float s, const Vector3f_a& v) {
	return Vector3f_a(s * v[0], s * v[1], s * v[2]);
}

static inline constexpr Vector3f_a operator/(float s, const Vector3f_a& v) {
	return Vector3f_a(s / v[0], s / v[1], s / v[2]);
}

static inline constexpr Vector3f_a operator-(const Vector3f_a& v) {
	return Vector3f_a(-v[0], -v[1], -v[2]);
}

static inline constexpr Vector3f_a& operator+=(Vector3f_a& a, const Vector3f_a& b) {
	a[0] += b[0];
	a[1] += b[1];
	a[2] += b[2];
	return a;
}

static inline constexpr Vector3f_a& operator-=(Vector3f_a& a, const Vector3f_a& b) {
	a[0] -= b[0];
	a[1] -= b[1];
	a[1] -= b[2];
	return a;
}

static inline constexpr Vector3f_a& operator*=(Vector3f_a& a, const Vector3f_a& b) {
	a[0] *= b[0];
	a[1] *= b[1];
	a[2] *= b[2];
	return a;
}

static inline constexpr Vector3f_a& operator*=(Vector3f_a& a, float s) {
	a[0] *= s;
	a[1] *= s;
	a[2] *= s;
	return a;
}

static inline constexpr Vector3f_a& operator/=(Vector3f_a& a, float s) {
	const float is = 1.f / s;
	a[0] *= is;
	a[1] *= is;
	a[2] *= is;
	return a;
}

static inline constexpr float dot(const Vector3f_a& a, const Vector3f_a& b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline float length(const Vector3f_a& v) {
	return std::sqrt(dot(v, v));
}

static inline constexpr float squared_length(const Vector3f_a& v) {
	return dot(v, v);
}

static inline Vector3f_a normalize(const Vector3f_a& v) {
	// This is slowest on both machines
//	return v / length(v);

	// This seems to give the best performance on clang & AMD
	return rsqrt(dot(v, v)) * v;
}

static inline Vector3f_a reciprocal(const Vector3f_a& v) {
//	return Vector3f_a(1.f / v[0], 1.f / v[1], 1.f / v[2]);

	Vector sx = simd::load_float4(v.v);

	Vector rcp = _mm_rcp_ps(sx);
	rcp = _mm_and_ps(rcp, simd::Mask3);
	Vector mul = _mm_mul_ps(sx, _mm_mul_ps(rcp, rcp));

	Vector3f_a result;
	simd::store_float4(result.v, _mm_sub_ps(_mm_add_ps(rcp, rcp), mul));
	return result;
}

static inline constexpr Vector3f_a cross(const Vector3f_a& a, const Vector3f_a& b) {
	return Vector3f_a(a[1] * b[2] - a[2] * b[1],
					  a[2] * b[0] - a[0] * b[2],
					  a[0] * b[1] - a[1] * b[0]);
}

static inline constexpr Vector3f_a project(const Vector3f_a& a, const Vector3f_a& b) {
	return dot(b, a) * b;
}

static inline float distance(const Vector3f_a& a, const Vector3f_a& b) {
	return length(a - b);
}

static inline constexpr float squared_distance(const Vector3f_a& a, const Vector3f_a& b) {
	return squared_length(a - b);
}

static inline constexpr Vector3f_a saturate(const Vector3f_a& v) {
	return Vector3f_a(std::clamp(v[0], 0.f, 1.f),
					  std::clamp(v[1], 0.f, 1.f),
					  std::clamp(v[2], 0.f, 1.f));
}

static inline Vector3f_a exp(const Vector3f_a& v) {
//	return Vector3f_a(math::exp(v[0]), math::exp(v[1]), math::exp(v[2]));
	Vector x = simd::load_float4(v.v);
	x = exp(x);

	Vector3f_a r;
	simd::store_float4(r.v, x);
	return r;
}

static inline Vector3f_a pow(const Vector3f_a& v, float e) {
	return Vector3f_a(std::pow(v[0], e),
					  std::pow(v[1], e),
					  std::pow(v[2], e));
}

static inline constexpr Vector3f_a lerp(const Vector3f_a& a, const Vector3f_a& b, float t) {
	const float u = 1.f - t;
	return u * a + t * b;
}

static inline constexpr Vector3f_a reflect(const Vector3f_a& normal, const Vector3f_a& v) {
	return 2.f * dot(v, normal) * normal - v;
}

static inline void orthonormal_basis(const Vector3f_a& n, Vector3f_a& t, Vector3f_a& b) {
	// https://gist.github.com/roxlu/3082114
/*
	// Handle the singularity
	if (n[2] < -0.9999999f) {
		t = Vector3f_a( 0.f, -1.f, 0.f);
		b = Vector3f_a(-1.f,  0.f, 0.f);
		return;
	}

	const float c = 1.f / (1.f + n[2]);
	const float d = -n[0] * n[1] * c;
	t = Vector3f_a(1.f - n[0] * n[0] * c, d, -n[0]);
	b = Vector3f_a(d, 1.f - n[1] * n[1] * c, -n[1]);
	*/

	// Building an Orthonormal Basis, Revisited
	// http://jcgt.org/published/0006/01/01/

//	const float sign = std::copysign(1.f, n[2]);
	const float sign = copysign1(n[2]);
	const float c = -1.f / (sign + n[2]);
	const float d = n[0] * n[1] * c;
	t = Vector3f_a(1.f + sign * n[0] * n[0] * c, sign * d, -sign * n[0]);
	b = Vector3f_a(d, sign + n[1] * n[1] * c, -n[1]);
}

// https://twitter.com/ian_mallett/status/846631289822232577
/*
static inline void orthonormal_basis_sse(const Vector3f_a& n, Vector3f_a& t, Vector3f_a& b) {
	const Vector u = simd::load_float3(n.v);

	const float sign = copysign1(n[2]);

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

static inline Vector3f_a tangent(const Vector3f_a& n) {
	const float sign = std::copysign(1.f, n[2]);
	const float c = -1.f / (sign + n[2]);
	const float d = n[0] * n[1] * c;
	return Vector3f_a(1.f + sign * n[0] * n[0] * c, sign * d, -sign * n[0]);
}

static inline constexpr Vector3f_a min(const Vector3f_a& a, float s) {
	return Vector3f_a(std::min(a[0], s), std::min(a[1], s), std::min(a[2], s));
}

static inline constexpr Vector3f_a min(const Vector3f_a& a, const Vector3f_a& b) {
	return Vector3f_a(std::min(a[0], b[0]), std::min(a[1], b[1]), std::min(a[2], b[2]));
}

static inline constexpr Vector3f_a max(const Vector3f_a& a, float s) {
	return Vector3f_a(std::max(a[0], s), std::max(a[1], s), std::max(a[2], s));
}

static inline constexpr Vector3f_a clamp(const Vector3f_a& v, float mi, float ma) {
	return Vector3f_a(std::min(std::max(v[0], mi), ma),
					  std::min(std::max(v[1], mi), ma),
					  std::min(std::max(v[2], mi), ma));
}

static inline constexpr Vector3f_a max(const Vector3f_a& a, const Vector3f_a& b) {
	return Vector3f_a(std::max(a[0], b[0]), std::max(a[1], b[1]), std::max(a[2], b[2]));
}

static inline constexpr float max_element(const Vector3f_a& v) {
	return std::max(std::max(v[0], v[1]), v[2]);
}

static inline constexpr float checksum(const Vector3f_a& v) {
	return v[0] + v[1] + v[2];
}

static inline Vector3f_a abs(const Vector3f_a& v) {
	return Vector3f_a(std::abs(v[0]), std::abs(v[1]), std::abs(v[2]));
}

static inline Vector3f_a cos(const Vector3f_a& v) {
	return Vector3f_a(std::cos(v[0]), std::cos(v[1]), std::cos(v[2]));
}

static inline Vector3f_a sqrt(const Vector3f_a& v) {
//	return Vector3f_a(std::sqrt(v[0]), std::sqrt(v[1]), std::sqrt(v[2]));

	Vector x = simd::load_float3(v.v);
	x = sqrt(x);

	Vector3f_a r;
	simd::store_float4(r.v, x);
	return r;
}

static inline Vector3f_a log(const Vector3f_a& v) {
//	return Vector3f_a(std::log(v[0]), std::log(v[1]), std::log(v[2]));

	Vector x = simd::load_float4(v.v);
	x = log(x);

	Vector3f_a r;
	simd::store_float4(r.v, x);
	return r;
}

static inline constexpr bool operator==(const Vector3f_a& a, const Vector3f_a& b) {
	return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

static inline constexpr bool operator!=(const Vector3f_a& a, const Vector3f_a& b) {
	return a[0] != b[0] || a[1] != b[1] || a[2] != b[2];
}

static inline constexpr bool all_lesser(const Vector3f_a& v, float s) {
	return v[0] < s && v[1] < s && v[2] < s;
}

static inline constexpr bool all_greater_equal(const Vector3f_a& v, float s) {
	return v[0] >= s && v[1] >= s && v[2] >= s;
}

static inline constexpr bool any_negative(const Vector3f_a& v) {
	return v[0] < 0.f || v[1] < 0.f || v[2] < 0.f;
}

static inline constexpr bool any_greater_zero(const Vector3f_a& v) {
	return v[0] > 0.f || v[1] > 0.f || v[2] > 0.f;
}

static inline constexpr bool any_greater_one(const Vector3f_a& v) {
	return v[0] > 1.f || v[1] > 1.f || v[2] > 1.f;
}

static inline constexpr bool any_lesser_one(const Vector3f_a& v) {
	return v[0] < 1.f || v[1] < 1.f || v[2] < 1.f;
}

static inline bool any_nan(const Vector3f_a& v) {
	return std::isnan(v[0]) || std::isnan(v[1]) || std::isnan(v[2]);
}

static inline bool any_inf(const Vector3f_a& v) {
	return std::isinf(v[0]) || std::isinf(v[1]) || std::isinf(v[2]);
}

static inline bool all_finite(const Vector3f_a& v) {
	return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]);
}

static inline bool all_finite_and_positive(const Vector3f_a& v) {
	return std::isfinite(v[0]) && v[0] >= 0.f
		&& std::isfinite(v[1]) && v[1] >= 0.f
		&& std::isfinite(v[2]) && v[2] >= 0.f;
}

}

#endif
