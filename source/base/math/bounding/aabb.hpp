#pragma once

#include "math/vector3.hpp"
#include "math/matrix4x4.hpp"
#include "math/ray.hpp"
#include "math/simd/simd_vector.hpp"

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

	T surface_area() const;
	T volume() const;

	bool intersect_p(const math::Optimized_ray<T>& ray) const;
	bool intersect_p(const math::Optimized_ray<T>& ray, T& min_t, T& max_t) const;

//	bool intersect_p(simd::FVector origin, simd::FVector inv_direction, float min_t, float max_t);

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
