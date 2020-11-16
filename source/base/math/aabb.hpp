#ifndef SU_BASE_MATH_AABB_HPP
#define SU_BASE_MATH_AABB_HPP

#include "math/matrix.hpp"
#include "math/vector3.hpp"

namespace math {

struct ray;

struct AABB {
    AABB();

    constexpr AABB(float3_p min, float3_p max);

    AABB(Simd3f_p min, Simd3f_p max);

    float3 min() const;
    float3 max() const;

    float3 position() const;

    float3 halfsize() const;

    float3 extent() const;

    float surface_area() const;

    float volume() const;

    bool intersect(float3_p p) const;

    bool intersect_p(ray const& ray) const;

    bool intersect_p(ray const& ray, float& hit_t) const;

    bool intersect_inside(ray const& ray, float& hit_t) const;

    float3 normal(float3_p p) const;

    void set_min_max(float3_p min, float3_p max);
    void set_min_max(Simd3f_p min, Simd3f_p max);

    void insert(float3_p p);

    void scale(float x);

    void add(float x);

    AABB transform(float4x4 const& m) const;

    AABB transform_transposed(float3x3 const& m) const;

    AABB merge(AABB const& other) const;

    AABB intersection(AABB const& other) const;

    void merge_assign(AABB const& other);

    void clip_min(float d, uint8_t axis);
    void clip_max(float d, uint8_t axis);

    bool operator==(AABB const& other) const;

    static constexpr AABB empty();

    static constexpr AABB infinite();

    float3 bounds[2];
};

struct Simd_AABB {
    Simd_AABB();
    Simd_AABB(AABB const& box);
    Simd_AABB(float const* min, float const* max);
    Simd_AABB(Simd3f_p min, Simd3f_p max);

    void merge_assign(Simd_AABB const& other);

    void merge_assign(Simd3f_p other_min, Simd3f_p other_max);

    Simd3f min;
    Simd3f max;
};

}  // namespace math

using AABB      = math::AABB;
using Simd_AABB = math::Simd_AABB;

#endif
