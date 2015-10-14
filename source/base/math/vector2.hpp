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
	explicit Vector2(Vector2<U> v);

	Vector2 yx() const;

	Vector2 operator+(Vector2 v) const;

	Vector2 operator-(Vector2 v) const;

	Vector2 operator*(Vector2 v) const;

	Vector2 operator/(T s) const;

	Vector2 &operator+=(Vector2 v);

	Vector2 &operator/=(T s);

	bool operator==(Vector2 v) const;

	bool operator!=(Vector2 v) const;

	static const Vector2 identity;
};

template<typename T>
Vector2<T> operator*(T s, Vector2<T> v);

template<typename T>
Vector2<T> operator/(T s, Vector2<T> v);

template<typename T>
T dot(const Vector2<T>& a, Vector2<T> b);

template<typename T>
T length(const Vector2<T>& v);

template<typename T>
T squared_length(Vector2<T> v);

template<typename T>
Vector2<T> normalized(Vector2<T> v);

template<typename T>
Vector2<T> reciprocal(Vector2<T> v);

template<typename T>
Vector2<T> lerp(Vector2<T> a, Vector2<T> b, T t);

template<typename T>
Vector2<T> round(Vector2<T> v);

template<typename T>
Vector2<T> min(Vector2<T> a, Vector2<T> b);

template<typename T>
Vector2<T> max(Vector2<T> a, Vector2<T> b);

template<typename T>
bool contains_nan(const Vector2<T>& v);

template<typename T>
bool contains_inf(const Vector2<T>& v);

}
