#pragma once

#include "math/vector3.hpp"
#include "math/matrix4x4.hpp"
#include "math/ray.hpp"

namespace math {

class AABB {

public:

	AABB();
	AABB(FVector3f_a min, FVector3f_a max);

	const Vector3f_a& min() const;
	const Vector3f_a& max() const;

	Vector3f_a position() const;
	Vector3f_a halfsize() const;

	float surface_area() const;
	float volume() const;

	bool intersect_p(const math::Oray& ray) const;
	bool intersect_p(const math::Oray& ray, float& min_t, float& max_t) const;

	void set_min_max(FVector3f_a min, FVector3f_a max);

	void insert(FVector3f_a p);

	AABB transform(const Matrix4x4f_a& m) const;

	AABB merge(const AABB& other) const;
	void merge_assign(const AABB& other);

	static AABB empty();
	static AABB infinite();

private:

	Vector3f_a bounds_[2];
};

using aabb = AABB;

}
