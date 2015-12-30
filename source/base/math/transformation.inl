#include "transformation.hpp"
#include "vector3.inl"
#include "quaternion.inl"

namespace math {

template<typename T>
Transformation<T> lerp(const Transformation<T>& a, const Transformation<T>& b, T t) {
	return Transformation<T>{
		lerp(a.position, b.position, t),
		lerp(a.scale, b.scale, t),
		slerp(a.rotation, b.rotation, t)
	};
}

}
