#ifndef SU_BASE_MATH_AABB_HPP
#define SU_BASE_MATH_AABB_HPP

#include "math/matrix.hpp"
#include "math/vector3.hpp"
#include "simd/simd.hpp"

namespace math {

struct Ray;

struct AABB {
    AABB() = default;
    constexpr AABB(float3 const& min, float3 const& max);
    AABB(FVector min, FVector max);

    float3 const& min() const;
    float3 const& max() const;

    float3 position() const;
    float3 halfsize() const;
    float3 extent() const;

    float surface_area() const;
    float volume() const;

    bool intersect(float3 const& p) const;

    bool intersect_p(Ray const& ray) const;

    bool intersect_p(FVector ray_origin, FVector ray_inv_direction, FVector ray_min_t,
                     FVector ray_max_t) const;

    bool intersect_p(Ray const& ray, float& hit_t) const;

    bool intersect_inside(Ray const& ray, float& hit_t) const;

    float3 normal(float3 const& p) const;

    void set_min_max(float3 const& min, float3 const& max);
    void set_min_max(FVector min, FVector max);

    void insert(float3 const& p);

    void scale(float x);
    void add(float x);

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

}  // namespace math

#endif
