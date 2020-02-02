#ifndef SU_BASE_MATH_AABB_HPP
#define SU_BASE_MATH_AABB_HPP

#include "math/matrix.hpp"
#include "math/vector3.hpp"

namespace math {

struct ray;

struct AABB {
    AABB();

    constexpr AABB(float3 const& min, float3 const& max);

    AABB(Simd3f const& min, Simd3f const& max);

    float3 const& min() const;
    float3 const& max() const;

    float3 position() const;

    float3 halfsize() const;

    float3 extent() const;

    float surface_area() const;

    float volume() const;

    bool intersect(float3 const& p) const;

    bool intersect_p(ray const& ray) const;

    bool intersect_p(ray const& ray, float& hit_t) const;

    bool intersect_inside(ray const& ray, float& hit_t) const;

    float3 normal(float3 const& p) const;

    void set_min_max(float3 const& min, float3 const& max);
    void set_min_max(Simd3f const& min, Simd3f const& max);

    void insert(float3 const& p);

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
    Simd_AABB(Simd3f const& min, Simd3f const& max);

    void merge_assign(Simd_AABB const& other);

    void merge_assign(Simd3f const& other_min, Simd3f const& other_max);

    Simd3f min;
    Simd3f max;
};

}  // namespace math

using AABB      = math::AABB;
using Simd_AABB = math::Simd_AABB;

#endif
