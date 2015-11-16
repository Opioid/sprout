#pragma once

#include "math/vector3.hpp"
#include "math/matrix4x4.hpp"
#include "math/ray.hpp"

namespace math {

template<typename T>
class AABB {
public:

	AABB();
	AABB(const Vector3<T>& min, const Vector3<T>& max);

	const Vector3<T>& min() const;
	const Vector3<T>& max() const;

	Vector3<T> position() const;
	Vector3<T> halfsize() const;

	T volume() const;

	bool intersect_p(const math::Optimized_ray<T>& ray) const;

	void set_min_max(const Vector3<T>& min, const Vector3<T>& max);

	void insert(const Vector3<T>& p);

	AABB transform(const Matrix4x4<T>& m) const;

	AABB merge(const AABB& other) const;
	void merge_assign(const AABB& other);

	static AABB empty();
	static AABB infinite();

private:

	Vector3<T> bounds_[2];
};

typedef AABB<float> aabb;

}
