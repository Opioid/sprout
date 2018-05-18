#ifndef SU_BASE_MATH_AABB_HPP
#define SU_BASE_MATH_AABB_HPP

#include "math/vector3.hpp"
#include "math/matrix.hpp"
#include "simd/simd.hpp"

namespace math {

struct Ray;
	
struct AABB {
	AABB() = default;
	constexpr AABB(f_float3 min, f_float3 max);
	AABB(FVector min, FVector max);

	f_float3 min() const;
	f_float3 max() const;

	float3 position() const;
	float3 halfsize() const;

	float surface_area() const;
	float volume() const;

	bool intersect(f_float3 p) const;

	bool intersect_p(Ray const& ray) const;

	bool intersect_p(FVector ray_origin, FVector ray_inv_direction,
					 FVector ray_min_t, FVector ray_max_t) const;

	bool intersect_p(Ray const& ray, float& hit_t) const;
	bool intersect_inside(Ray const& ray, float& hit_t) const;

	void set_min_max(f_float3 min, f_float3 max);
	void set_min_max(FVector min, FVector max);

	void insert(f_float3 p);

	AABB transform(const Matrix4x4f_a& m) const;

	AABB merge(AABB const& other) const;
	void merge_assign(AABB const& other);

	void clip_min(float d, uint8_t axis);
	void clip_max(float d, uint8_t axis);

	bool operator==(AABB const& other) const;

	static constexpr AABB empty();
	static constexpr AABB infinite();

	float3 bounds[2];
};

}

#endif
