#pragma once

namespace math {

template<typename T>
struct Vector2 {
	union {
		struct {
			T x, y;
		};

		T v[2];
	};

	Vector2();

	Vector2(T x, T y);

	template<typename U>
	explicit Vector2(const Vector2<U>& v);

	Vector2 operator+(const Vector2& v) const;

	Vector2 operator-(const Vector2& v) const;

	Vector2 operator*(const Vector2& v) const;

	Vector2 operator/(T s) const;

	Vector2 &operator+=(const Vector2& v);

	Vector2 &operator/=(T s);

	bool operator==(const Vector2& v) const;

	bool operator!=(const Vector2& v) const;

	static const Vector2 identity;
};

template<typename T>
inline Vector2<T> operator*(T s, const Vector2<T>& v);

template<typename T>
inline Vector2<T> operator/(T s, const Vector2<T>& v);

template<typename T>
inline T dot(const Vector2<T>& a, const Vector2<T>& b);

template<typename T>
inline T length(const Vector2<T>& v);

template<typename T>
inline T squared_length(const Vector2<T>& v);

template<typename T>
inline Vector2<T> normalize(const Vector2<T>& v);

template<typename T>
inline Vector2<T> reciprocal(const Vector2<T>& v);

template<typename T>
inline Vector2<T> round(const Vector2<T>& v);

}
