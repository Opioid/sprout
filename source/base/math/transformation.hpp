#pragma once

namespace math {

template<typename T> struct Vector3;
template<typename T> struct Quaternion;

template<typename T>
struct Transformation {
	Vector3<T>    position;
	Vector3<T>    scale;
	Quaternion<T> rotation;
};

typedef Transformation<float> transformation;

}
