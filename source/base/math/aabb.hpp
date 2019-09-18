#ifndef SU_BASE_MATH_AABB_HPP
#define SU_BASE_MATH_AABB_HPP

#include "math/matrix.hpp"
#include "math/vector3.hpp"

namespace math {

struct ray;

struct AABB {
    AABB() noexcept;

    constexpr AABB(float3 const& min, float3 const& max) noexcept;

    AABB(Simd3f const& min, Simd3f const& max) noexcept;

    float3 const& min() const noexcept;
    float3 const& max() const noexcept;

    float3 position() const noexcept;

    float3 halfsize() const noexcept;

    float3 extent() const noexcept;

    float surface_area() const noexcept;

    float volume() const noexcept;

    bool intersect(float3 const& p) const noexcept;

    bool intersect_p(ray const& ray) const noexcept;

    bool intersect_p(ray const& ray, float& hit_t) const noexcept;

    bool intersect_inside(ray const& ray, float& hit_t) const noexcept;

    float3 normal(float3 const& p) const noexcept;

    void set_min_max(float3 const& min, float3 const& max) noexcept;
    void set_min_max(Simd3f const& min, Simd3f const& max) noexcept;

    void insert(float3 const& p) noexcept;

    void scale(float x) noexcept;

    void add(float x) noexcept;

    AABB transform(float4x4 const& m) const noexcept;

    AABB transform_transposed(float3x3 const& m) const noexcept;

    AABB merge(AABB const& other) const noexcept;

    void merge_assign(AABB const& other) noexcept;

    void clip_min(float d, uint8_t axis) noexcept;
    void clip_max(float d, uint8_t axis) noexcept;

    bool operator==(AABB const& other) const noexcept;

    static constexpr AABB empty() noexcept;

    static constexpr AABB infinite() noexcept;

    float3 bounds[2];
};

struct Simd_AABB {
    Simd_AABB() noexcept;
    Simd_AABB(AABB const& box) noexcept;
    Simd_AABB(float const* min, float const* max) noexcept;
    Simd_AABB(Simd3f const& min, Simd3f const& max) noexcept;

    void merge_assign(Simd_AABB const& other) noexcept;

    void merge_assign(Simd3f const& other_min, Simd3f const& other_max) noexcept;

    Simd3f min;
    Simd3f max;
};

}  // namespace math

using AABB      = math::AABB;
using Simd_AABB = math::Simd_AABB;

#endif
