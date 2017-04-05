#pragma once

#include "math/vector3.hpp"
#include "math/matrix4x4.hpp"
#include "math/ray.hpp"
#include "simd/simd.hpp"

namespace math {

class AABB {

public:

	AABB() = default;
	constexpr AABB(const Vector3f_a& min, const Vector3f_a& max);
	AABB(FVector min, FVector max);

	const Vector3f_a& min() const;
	const Vector3f_a& max() const;

	Vector3f_a position() const;
	Vector3f_a halfsize() const;

	float surface_area() const;
	float volume() const;

	bool intersect(const Vector3f_a& p) const;

	bool intersect_p(const Ray& ray) const;
	bool intersect_p(const Ray& ray, float& min_t, float& max_t) const;

	void set_min_max(const Vector3f_a& min, const Vector3f_a& max);
	void set_min_max(FVector min, FVector max);

	void insert(const Vector3f_a& p);

	AABB transform(const Matrix4x4f_a& m) const;

	AABB merge(const AABB& other) const;
	void merge_assign(const AABB& other);

	void clip_min(float d, uint8_t axis);
	void clip_max(float d, uint8_t axis);

	static constexpr AABB empty();
	static constexpr AABB infinite();

private:

	Vector3f_a bounds_[2];
};

}
