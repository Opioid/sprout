#ifndef SU_BASE_MATH_AABB_HPP
#define SU_BASE_MATH_AABB_HPP

#include "math/matrix.hpp"
#include "math/vector3.hpp"

namespace math {

struct ray;
struct Simd_AABB;

struct AABB {
    AABB();

    constexpr AABB(float3_p min, float3_p max);

    explicit AABB(Simd_AABB const& box);
    AABB(Simdf_p min, Simdf_p max);

    float3 min() const;
    float3 max() const;

    float3 position() const;

    float3 halfsize() const;

    float3 extent() const;

    float surface_area() const;

    float volume() const;

    float cached_radius() const;

    bool intersect(float3_p p) const;

    bool intersect_p(ray const& ray) const;

    bool intersect_p(ray const& ray, float& hit_t) const;

    bool intersect_inside(ray const& ray, float& hit_t) const;

    float3 normal(float3_p p) const;

    void insert(float3_p p);

    void scale(float x);

    void add(float x);

    void cache_radius();

    AABB transform(float4x4 const& m) const;

    AABB transform_transposed(float3x3 const& m) const;

    AABB merge(AABB const& other) const;

    AABB intersection(AABB const& other) const;

    void merge_assign(AABB const& other);

    void clip_min(float d, uint8_t axis);
    void clip_max(float d, uint8_t axis);

    bool covers(AABB const& other) const;

    float3 bounds[2];
};

struct Simd_AABB {
    Simd_AABB();
    Simd_AABB(AABB const& box);
    Simd_AABB(float const* min, float const* max);
    Simd_AABB(Simdf_p min, Simdf_p max);

    void merge_assign(Simd_AABB const& other);

    void merge_assign(Simdf_p other_min, Simdf_p other_max);

    Simdf min;
    Simdf max;
};

}  // namespace math

using AABB      = math::AABB;
using Simd_AABB = math::Simd_AABB;

#endif
