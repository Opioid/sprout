#pragma once

#include "math/vector3.hpp"
#include "math/matrix4x4.hpp"
#include "math/ray.hpp"
#include "math/simd/simd_vector.hpp"

namespace math {

class AABB {
public:

	AABB();
	AABB(const Vector3f_a& min, const Vector3f_a& max);

	const Vector3f_a& min() const;
	const Vector3f_a& max() const;

	Vector3f_a position() const;
	Vector3f_a halfsize() const;

	float surface_area() const;
	float volume() const;

	bool intersect_p(const math::Oray& ray) const;
	bool intersect_p(const math::Oray& ray, float& min_t, float& max_t) const;

	bool SU_CALLCONV intersect_p(simd::FVector origin, simd::FVector inv_direction, float min_t, float max_t);

	void set_min_max(const Vector3f_a& min, const Vector3f_a& max);

	void insert(const Vector3f_a& p);

	AABB transform(const Matrix4x4f_a& m) const;

	AABB merge(const AABB& other) const;
	void merge_assign(const AABB& other);

	static AABB empty();
	static AABB infinite();

private:

	Vector3f_a bounds_[2];
};

typedef AABB aabb;

}
