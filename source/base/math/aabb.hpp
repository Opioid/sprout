#ifndef SU_BASE_MATH_AABB_HPP
#define SU_BASE_MATH_AABB_HPP

#include "math/vector3.hpp"
#include "math/matrix4x4.hpp"
#include "math/ray.hpp"
#include "simd/simd.hpp"

namespace math {

class AABB {

public:

	AABB() = default;
	constexpr AABB(const float3& min, const float3& max);
	AABB(VVector min, VVector max);

	const float3& min() const;
	const float3& max() const;

	Vector3f_a position() const;
	Vector3f_a halfsize() const;

	float surface_area() const;
	float volume() const;

	bool intersect(const float3& p) const;

	bool intersect_p(const Ray& ray) const;

	bool intersect_p(VVector ray_origin, VVector ray_inv_direction,
					 VVector ray_min_t, VVector ray_max_t) const;

	bool intersect_p(const Ray& ray, float& min_t, float& max_t) const;
	bool intersect_p(const Ray& ray, float& hit_t) const;

	void set_min_max(const float3& min, const float3& max);
	void set_min_max(VVector min, VVector max);

	void insert(const float3& p);

	AABB transform(const Matrix4x4f_a& m) const;

	AABB merge(const AABB& other) const;
	void merge_assign(const AABB& other);

	void clip_min(float d, uint8_t axis);
	void clip_max(float d, uint8_t axis);

	bool operator==(const AABB& other) const;

	static constexpr AABB empty();
	static constexpr AABB infinite();

private:

	float3 bounds_[2];
};

}

#endif
