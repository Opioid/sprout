#ifndef SU_BASE_MATH_AABB_HPP
#define SU_BASE_MATH_AABB_HPP

#include "math/vector3.hpp"
#include "math/matrix.hpp"
#include "simd/simd.hpp"

namespace math {

struct Ray;
	
class AABB {

public:

	AABB() = default;
	constexpr AABB(const float3& min, const float3& max);
	AABB(FVector min, FVector max);

	const float3& min() const;
	const float3& max() const;

	float3 position() const;
	float3 halfsize() const;

	float surface_area() const;
	float volume() const;

	bool intersect(f_float3 p) const;

	bool intersect_p(const Ray& ray) const;

	bool intersect_p(FVector ray_origin, FVector ray_inv_direction,
					 FVector ray_min_t, FVector ray_max_t) const;

	bool intersect_p(const Ray& ray, float& min_t, float& max_t) const;
	bool intersect_p(const Ray& ray, float& hit_t, bool& inside) const;

	void set_min_max(const float3& min, const float3& max);
	void set_min_max(FVector min, FVector max);

	void insert(f_float3 p);

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
