#pragma once

#include "vector2.hpp"
#include <cstdint>

namespace math {

struct Vector3f_a;

using FVector3f_a = const Vector3f_a&;

/****************************************************************************
 *
 * Generic 3D vector
 *
 ****************************************************************************/

template<typename T>
struct Vector3 {
	union {
		struct {
			T x, y, z;
		};

		Vector2<T> xy;

		T v[3];
	};

	Vector3();

	Vector3(T x, T y, T z);

	explicit Vector3(T s);

	explicit Vector3(Vector2<T> xy, T z = T(0));

	explicit Vector3(const T* v);

	explicit Vector3(FVector3f_a a);

	Vector3 operator+(T s) const;

	Vector3 operator+(const Vector3& a) const;

	Vector3 operator-(T s) const;

	Vector3 operator-(const Vector3& v) const;
		
	Vector3 operator*(const Vector3& a) const;
		
	Vector3 operator/(T s) const;

	Vector3 operator/(const Vector3& v) const;

	Vector3 operator-() const;

	Vector3& operator+=(const Vector3& a);

	Vector3& operator-=(const Vector3& a);

	Vector3& operator*=(const Vector3& a);

	Vector3& operator*=(T s);

	Vector3& operator/=(T s);

	bool operator==(const Vector3& a) const;

	bool operator!=(const Vector3& a) const;

//	explicit operator unsigned int() const;

	T absolute_max(uint32_t& i) const;

	static const Vector3 identity;
};

template<typename T>
Vector3<T> operator*(T s, const Vector3<T>& v);

template<typename T>
T dot(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
T length(const Vector3<T>& v);

template<typename T>
T squared_length(const Vector3<T>& v);

template<typename T>
Vector3<T> normalized(const Vector3<T>& v);

template<typename T>
Vector3<T> reciprocal(const Vector3<T>& v);

template<typename T>
Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
Vector3<T> project(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
T distance(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
T squared_distance(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
Vector3<T> saturate(const Vector3<T>& v);

template<typename T>
Vector3<T> exp(const Vector3<T>& v);

template<typename T>
Vector3<T> lerp(const Vector3<T>& a, const Vector3<T>& b, T t);

template<typename T>
Vector3<T> reflect(const Vector3<T>& normal, const Vector3<T>& v);

// Assuming n is unit length
template<typename T>
void coordinate_system(const Vector3<T>& n, Vector3<T>& t, Vector3<T>& b);

template<typename T>
Vector3<T> min(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
Vector3<T> max(const Vector3<T>& a, const Vector3<T>& b);

template<typename T>
Vector3<T> abs(const Vector3<T>& v);

template<typename T>
bool contains_negative(const Vector3<T>& v);

template<typename T>
bool contains_greater_one(const Vector3<T>& v);

template<typename T>
bool contains_nan(const Vector3<T>& v);

template<typename T>
bool contains_inf(const Vector3<T>& v);

/****************************************************************************
 *
 * Aligned 3D float vector
 *
 ****************************************************************************/

struct alignas(16) Vector3f_a {
	union {
		struct {
			float x, y, z;
		};

		Vector2<float> xy;

		// 4 instead of 3 in order to hide pad warning
		float v[4];
	};

	Vector3f_a();

	Vector3f_a(float x, float y, float z);

	explicit Vector3f_a(float s);

	explicit Vector3f_a(Vector2<float> xy, float z);

	template<typename T>
	explicit Vector3f_a(const Vector3<T>& v);

	Vector3f_a operator+(float s) const;

	Vector3f_a operator+(FVector3f_a v) const;

	Vector3f_a operator-(float s) const;

	Vector3f_a operator-(FVector3f_a v) const;

	Vector3f_a operator*(FVector3f_a v) const;

	Vector3f_a operator/(float s) const;

	Vector3f_a operator/(FVector3f_a v) const;

	Vector3f_a operator-() const;

	Vector3f_a& operator+=(FVector3f_a v);

	Vector3f_a& operator-=(FVector3f_a v);

	Vector3f_a& operator*=(FVector3f_a v);

	Vector3f_a& operator*=(float s);

	Vector3f_a& operator/=(float s);

	bool operator==(FVector3f_a v) const;

	bool operator!=(FVector3f_a v) const;

	float absolute_max(uint32_t& i) const;
};

Vector3f_a operator+(float s, FVector3f_a v);

Vector3f_a operator-(float s, FVector3f_a v);

Vector3f_a operator*(float s, FVector3f_a v);

Vector3f_a operator/(float s, FVector3f_a v);

Vector3f_a mul(FVector3f_a v, float s);

float dot(FVector3f_a a, FVector3f_a b);

float length(FVector3f_a v);

float squared_length(FVector3f_a v);

Vector3f_a normalized(FVector3f_a v);

Vector3f_a reciprocal(FVector3f_a v);

Vector3f_a cross(FVector3f_a a, FVector3f_a b);

Vector3f_a project(FVector3f_a a, FVector3f_a b);

float distance(FVector3f_a a, FVector3f_a b);

float squared_distance(FVector3f_a a, FVector3f_a b);

Vector3f_a saturate(FVector3f_a v);

Vector3f_a exp(FVector3f_a v);

Vector3f_a pow(FVector3f_a v, float e);

Vector3f_a lerp(FVector3f_a a, FVector3f_a b, float t);

Vector3f_a reflect(FVector3f_a normal, FVector3f_a v);

// Assuming n is unit length
void coordinate_system(FVector3f_a n, Vector3f_a& t, Vector3f_a& b);
Vector3f_a tangent(FVector3f_a n);

Vector3f_a min(FVector3f_a a, FVector3f_a b);

Vector3f_a max(FVector3f_a a, FVector3f_a b);

float max_element(FVector3f_a v);

Vector3f_a abs(FVector3f_a v);

Vector3f_a cos(FVector3f_a v);

bool all_lesser(FVector3f_a v, float s);

bool all_greater_equal(FVector3f_a v, float s);

bool contains_negative(FVector3f_a v);

bool contains_greater_zero(FVector3f_a v);

bool contains_greater_one(FVector3f_a v);

bool contains_nan(FVector3f_a v);

bool contains_inf(FVector3f_a v);

bool contains_only_finite(FVector3f_a v);

}
