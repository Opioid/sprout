#pragma once

namespace math {

template<typename T> struct Vector3;
template<typename T> struct Matrix4x4;
template<typename T> struct Quaternion;

template<typename T> 
struct Matrix3x3 {
	union {
		struct {
			T m00, m01, m02,
			  m10, m11, m12, 
			  m20, m21, m22;
		};

		struct {
			T m[9]; 
		};

		struct {
			Vector3<T> rows[3];
		};

		struct {
			Vector3<T> x, y, z;
		};
	};

	Matrix3x3();

	Matrix3x3(T m00, T m01, T m02,
			  T m10, T m11, T m12,
			  T m20, T m21, T m22);
		
	Matrix3x3(const Vector3<T>& x, const Vector3<T>& y, const Vector3<T>& z);
	
	explicit Matrix3x3(const Matrix4x4<T>& m);

	explicit Matrix3x3(const Quaternion<T>& q);

	Matrix3x3 operator*(const Matrix3x3& m) const;

	Matrix3x3 operator/(T s) const;

	Matrix3x3& operator*=(const Matrix3x3& m);

	static const Matrix3x3 identity;
};

template<typename T>
Vector3<T> operator*(const Vector3<T>& v, const Matrix3x3<T>& m);

template<typename T>
Vector3<T>& operator*=(Vector3<T>& v, const Matrix3x3<T>& m);

template<typename T>
Vector3<T> operator*(const Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
Vector3<T> transform_vector(const Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
void transform_vectors(const Matrix3x3<T>& m, const Vector3<T>& a, const Vector3<T>& b, Vector3<T>& oa, Vector3<T>& ob);

template<typename T>
Matrix3x3<T> normalize(const Matrix3x3<T>& m);

template<typename T>
T det(const Matrix3x3<T>& m);

template<typename T>
Matrix3x3<T> invert(const Matrix3x3<T>& m);

template<typename T>
inline void set_look_at(Matrix3x3<T>& m, const Vector3<T>& dir, const Vector3<T>& up);

template<typename T>
void set_basis(Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
void set_scale(Matrix3x3<T>& m, T x, T y, T z);

template<typename T>
void set_scale(Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
void scale(Matrix3x3<T>& m, const Vector3<T>& v);

template<typename T>
void set_rotation_x(Matrix3x3<T>& m, T a);

template<typename T>
void set_rotation_y(Matrix3x3<T>& m, T a);

template<typename T>
void set_rotation_z(Matrix3x3<T>& m, T a);

template<typename T>
void set_rotation(Matrix3x3<T>& m, const Vector3<T>& v, T a);

template<typename T>
Matrix3x3<T> transposed(const Matrix3x3<T>& m);

}
