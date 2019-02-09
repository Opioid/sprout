#ifndef SU_BASE_MATH_AABB_HPP
#define SU_BASE_MATH_AABB_HPP

#include "math/matrix.hpp"
#include "math/vector3.hpp"
#include "simd/simd.hpp"

namespace math {

struct ray;

struct AABB {
    AABB() noexcept = default;

    constexpr AABB(float3 const& min, float3 const& max) noexcept;

    AABB(FVector min, FVector max) noexcept;

    float3 const& min() const noexcept;
    float3 const& max() const noexcept;

    float3 position() const noexcept;
    float3 halfsize() const noexcept;
    float3 extent() const noexcept;

    float surface_area() const noexcept;

    float volume() const noexcept;

    bool intersect(float3 const& p) const noexcept;

    bool intersect_p(ray const& ray) const noexcept;

    bool intersect_p(FVector ray_origin, FVector ray_inv_direction, FVector ray_min_t,
                     FVector ray_max_t) const noexcept;

    bool intersect_p(ray const& ray, float& hit_t) const noexcept;

    bool intersect_inside(ray const& ray, float& hit_t) const noexcept;

    float3 normal(float3 const& p) const noexcept;

    void set_min_max(float3 const& min, float3 const& max) noexcept;
    void set_min_max(FVector min, FVector max) noexcept;

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

}  // namespace math

using AABB = math::AABB;

#endif
