#ifndef SU_BASE_MATH_AABB_INL
#define SU_BASE_MATH_AABB_INL

#include "aabb.hpp"
#include "matrix4x4.inl"
#include "ray.inl"
#include "vector3.inl"

namespace math {

inline AABB::AABB() = default;

inline constexpr AABB::AABB(float3_p min, float3_p max) : bounds{min, max} {}

inline AABB::AABB(Simd_AABB const& box) : bounds{float3(box.min), float3(box.max)} {}

inline AABB::AABB(Simdf_p min, Simdf_p max) : bounds{float3(min), float3(max)} {}

inline float3 AABB::min() const {
    return bounds[0];
}

inline float3 AABB::max() const {
    return bounds[1];
}

inline float3 AABB::position() const {
    return 0.5f * (bounds[0] + bounds[1]);
}

inline float3 AABB::halfsize() const {
    return 0.5f * (bounds[1] - bounds[0]);
}

inline float3 AABB::extent() const {
    return bounds[1] - bounds[0];
}

inline float AABB::surface_area() const {
    float3 const d = bounds[1] - bounds[0];
    return 2.f * (d[0] * d[1] + d[0] * d[2] + d[1] * d[2]);
}

inline float AABB::volume() const {
    float3 const d = bounds[1] - bounds[0];
    return d[0] * d[1] * d[2];
}

inline float AABB::cached_radius() const {
    return bounds[0][3];
}

inline bool AABB::intersect(float3_p p) const {
    if (p[0] >= bounds[0][0] && p[0] <= bounds[1][0] && p[1] >= bounds[0][1] &&
        p[1] <= bounds[1][1] && p[2] >= bounds[0][2] && p[2] <= bounds[1][2]) {
        return true;
    }

    return false;
}

// This test is presented in the paper
// "An Efficient and Robust ray–Box Intersection Algorithm"
// http://www.cs.utah.edu/~awilliam/box/box.pdf
inline bool AABB::intersect_p(ray const& ray) const {
    /*	int8_t sign_0 = ray.signs[0];
            float min_t = (bounds[    sign_0][0] - ray.origin[0]) * ray.inv_direction[0];
            float max_t = (bounds[1 - sign_0][0] - ray.origin[0]) * ray.inv_direction[0];

            int8_t sign_1 = ray.signs[1];
            float min_ty = (bounds[    sign_1][1] - ray.origin[1]) * ray.inv_direction[1];
            float max_ty = (bounds[1 - sign_1][1] - ray.origin[1]) * ray.inv_direction[1];

            if (min_t > max_ty || min_ty > max_t) {
                    return false;
            }

            if (min_ty > min_t) {
                    min_t = min_ty;
            }

            if (max_ty < max_t) {
                    max_t = max_ty;
            }

            int8_t sign_2 = ray.signs[2];
            float min_tz = (bounds[    sign_2][2] - ray.origin[2]) * ray.inv_direction[2];
            float max_tz = (bounds[1 - sign_2][2] - ray.origin[2]) * ray.inv_direction[2];

            if (min_t > max_tz || min_tz > max_t) {
                    return false;
            }

            if (min_tz > min_t) {
                    min_t = min_tz;
            }

            if (max_tz < max_t) {
                    max_t = max_tz;
            }

            return min_t < ray.max_t() && max_t > ray.min_t();*/

    Simdf const  ray_origin(ray.origin);
    Simdf const  ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar const ray_max_t(ray.max_t());

    Simdf const bb_min(bounds[0]);
    Simdf const bb_max(bounds[1]);

    Simdf const l1 = (bb_min - ray_origin) * ray_inv_direction;
    Simdf const l2 = (bb_max - ray_origin) * ray_inv_direction;

    // the order we use for those min/max is vital to filter out
    // NaNs that happens when an inv_dir is +/- inf and
    // (box_min - pos) is 0. inf * 0 = NaN
    Simdf const filtered_l1a = math::min(l1, Simdf(simd::Infinity));
    Simdf const filtered_l2a = math::min(l2, Simdf(simd::Infinity));

    Simdf const filtered_l1b = math::max(l1, Simdf(simd::Neg_infinity));
    Simdf const filtered_l2b = math::max(l2, Simdf(simd::Neg_infinity));

    // now that we're back on our feet, test those slabs.
    Simdf max_t = math::max(filtered_l1a, filtered_l2a);
    Simdf min_t = math::min(filtered_l1b, filtered_l2b);

    // unfold back. try to hide the latency of the shufps & co.
    max_t = min_scalar(max_t, SU_ROTATE_LEFT(max_t.v));
    min_t = max_scalar(min_t, SU_ROTATE_LEFT(min_t.v));

    max_t = min_scalar(max_t, SU_MUX_HIGH(max_t.v, max_t.v));
    min_t = max_scalar(min_t, SU_MUX_HIGH(min_t.v, min_t.v));

    return 0 != (_mm_comige_ss(max_t.v, ray_min_t.v) & _mm_comige_ss(ray_max_t.v, min_t.v) &
                 _mm_comige_ss(max_t.v, min_t.v));
}

inline bool AABB::intersect_p(ray const& ray, float& hit_t) const {
    Simdf const  ray_origin(ray.origin);
    Simdf const  ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar const ray_max_t(ray.max_t());

    Simdf const bb_min(bounds[0]);
    Simdf const bb_max(bounds[1]);

    Simdf const l1 = (bb_min - ray_origin) * ray_inv_direction;
    Simdf const l2 = (bb_max - ray_origin) * ray_inv_direction;

    // the order we use for those min/max is vital to filter out
    // NaNs that happens when an inv_dir is +/- inf and
    // (box_min - pos) is 0. inf * 0 = NaN
    Simdf const filtered_l1a = math::min(l1, Simdf(simd::Infinity));
    Simdf const filtered_l2a = math::min(l2, Simdf(simd::Infinity));

    Simdf const filtered_l1b = math::max(l1, Simdf(simd::Neg_infinity));
    Simdf const filtered_l2b = math::max(l2, Simdf(simd::Neg_infinity));

    // now that we're back on our feet, test those slabs.
    Simdf max_t = math::max(filtered_l1a, filtered_l2a);
    Simdf min_t = math::min(filtered_l1b, filtered_l2b);

    // unfold back. try to hide the latency of the shufps & co.
    max_t = min_scalar(max_t, SU_ROTATE_LEFT(max_t.v));
    min_t = max_scalar(min_t, SU_ROTATE_LEFT(min_t.v));

    max_t = min_scalar(max_t, SU_MUX_HIGH(max_t.v, max_t.v));
    min_t = max_scalar(min_t, SU_MUX_HIGH(min_t.v, min_t.v));

    float const min_out = min_t.x();
    float const max_out = max_t.x();

    if (min_out < ray.min_t()) {
        hit_t = max_out;
    } else {
        hit_t = min_out;
    }

    return 0 != (_mm_comige_ss(max_t.v, ray_min_t.v) & _mm_comige_ss(ray_max_t.v, min_t.v) &
                 _mm_comige_ss(max_t.v, min_t.v));
}

inline bool AABB::intersect_inside(ray const& ray, float& hit_t) const {
    Simdf const  ray_origin(ray.origin);
    Simdf const  ray_inv_direction(ray.inv_direction.v);
    scalar const ray_min_t(ray.min_t());
    scalar const ray_max_t(ray.max_t());

    Simdf const bb_min(bounds[0]);
    Simdf const bb_max(bounds[1]);

    Simdf const l1 = (bb_min - ray_origin) * ray_inv_direction;
    Simdf const l2 = (bb_max - ray_origin) * ray_inv_direction;

    // the order we use for those min/max is vital to filter out
    // NaNs that happens when an inv_dir is +/- inf and
    // (box_min - pos) is 0. inf * 0 = NaN
    Simdf const filtered_l1a = math::min(l1, Simdf(simd::Infinity));
    Simdf const filtered_l2a = math::min(l2, Simdf(simd::Infinity));

    Simdf const filtered_l1b = math::max(l1, Simdf(simd::Neg_infinity));
    Simdf const filtered_l2b = math::max(l2, Simdf(simd::Neg_infinity));

    // now that we're back on our feet, test those slabs.
    Simdf max_t = math::max(filtered_l1a, filtered_l2a);
    Simdf min_t = math::min(filtered_l1b, filtered_l2b);

    // unfold back. try to hide the latency of the shufps & co.
    max_t = min_scalar(max_t, SU_ROTATE_LEFT(max_t.v));
    min_t = max_scalar(min_t, SU_ROTATE_LEFT(min_t.v));

    max_t = min_scalar(max_t, SU_MUX_HIGH(max_t.v, max_t.v));
    min_t = max_scalar(min_t, SU_MUX_HIGH(min_t.v, min_t.v));

    float const min_out = min_t.x();
    float const max_out = max_t.x();

    if (min_out < ray.min_t()) {
        hit_t = max_out;
    } else {
        hit_t = min_out;
    }

    return 0 != (_mm_comige_ss(max_t.v, ray_min_t.v) & _mm_comige_ss(ray_min_t.v, min_t.v) &
                 _mm_comige_ss(ray_max_t.v, min_t.v) & _mm_comige_ss(max_t.v, min_t.v));
}

inline float3 AABB::normal(float3_p p) const {
    float3 const lp   = p - position();
    float3 const size = halfsize();
    float3 const dist = abs(size - abs(lp));

    uint32_t const i = index_min_component(dist);

    float3 normal(0.f);
    normal[i] = copysign1(lp[i]);

    return normal;
}

inline void AABB::insert(float3_p p) {
    bounds[0] = math::min(p, bounds[0]);
    bounds[1] = math::max(p, bounds[1]);
}

inline void AABB::scale(float x) {
    float3 const v = x * halfsize();
    bounds[0] -= v;
    bounds[1] += v;
}

inline void AABB::add(float x) {
    float3 const v(x);
    bounds[0] -= v;
    bounds[1] += v;
}

inline void AABB::cache_radius() {
    bounds[0][3] = 0.5f * length(extent());
}

inline AABB AABB::transform(float4x4 const& m) const {
    float3 const mx = m.x();
    float3 const xa = bounds[0][0] * mx;
    float3 const xb = bounds[1][0] * mx;

    float3 const my = m.y();
    float3 const ya = bounds[0][1] * my;
    float3 const yb = bounds[1][1] * my;

    float3 const mz = m.z();
    float3 const za = bounds[0][2] * mz;
    float3 const zb = bounds[1][2] * mz;

    float3 const mw = m.w();

    return AABB((math::min(xa, xb) + math::min(ya, yb)) + (math::min(za, zb) + mw),
                (math::max(xa, xb) + math::max(ya, yb)) + (math::max(za, zb) + mw));
}

inline AABB AABB::transform_transposed(float3x3 const& m) const {
    /*
        float3 const mx(m.r[0][0], m.r[1][0], m.r[2][0]);
        float3 const xa = bounds[0][0] * mx;
        float3 const xb = bounds[1][0] * mx;

        float3 const my(m.r[0][1], m.r[1][1], m.r[2][1]);
        float3 const ya = bounds[0][1] * my;
        float3 const yb = bounds[1][1] * my;

        float3 const mz(m.r[0][2], m.r[1][2], m.r[2][2]);
        float3 const za = bounds[0][2] * mz;
        float3 const zb = bounds[1][2] * mz;

        return AABB(math::min(xa, xb) + math::min(ya, yb) + math::min(za, zb),
                    math::max(xa, xb) + math::max(ya, yb) + math::max(za, zb));
    */

    float3 const mx(m.r[0][0], m.r[1][0], m.r[2][0]);
    float3 const xa = bounds[0][0] * mx;
    float3 const xb = bounds[1][0] * mx;

    float3 const my(m.r[0][1], m.r[1][1], m.r[2][1]);
    float3 const ya = bounds[0][1] * my;
    float3 const yb = bounds[1][1] * my;

    float3 const mz(m.r[0][2], m.r[1][2], m.r[2][2]);
    float3 const za = bounds[0][2] * mz;
    float3 const zb = bounds[1][2] * mz;

    float3 const min = math::min(xa, xb) + math::min(ya, yb) + math::min(za, zb);
    float3 const max = math::max(xa, xb) + math::max(ya, yb) + math::max(za, zb);

    float3 const halfsize = 0.5f * (max - min);

    float3 const p = position();

    return AABB(p - halfsize, p + halfsize);
}

inline AABB AABB::merge(AABB const& other) const {
    return AABB(math::min(bounds[0], other.bounds[0]), math::max(bounds[1], other.bounds[1]));
}

inline AABB AABB::intersection(AABB const& other) const {
    return AABB(math::max(bounds[0], other.bounds[0]), math::min(bounds[1], other.bounds[1]));
}

inline void AABB::merge_assign(AABB const& other) {
    bounds[0] = math::min(bounds[0], other.bounds[0]);
    bounds[1] = math::max(bounds[1], other.bounds[1]);
}

inline void AABB::clip_min(float d, uint8_t axis) {
    bounds[0].v[axis] = std::max(d, bounds[0][axis]);
}

inline void AABB::clip_max(float d, uint8_t axis) {
    bounds[1].v[axis] = std::min(d, bounds[1][axis]);
}

inline bool AABB::operator==(AABB const& other) const {
    return bounds[0] == other.bounds[0] && bounds[1] == other.bounds[1];
}

inline AABB constexpr Empty_AABB(float3(std::numeric_limits<float>::max()),
                                 float3(-std::numeric_limits<float>::max()));

inline AABB constexpr Infinite_AABB(float3(-std::numeric_limits<float>::max()),
                                    float3(std::numeric_limits<float>::max()));

inline Simd_AABB::Simd_AABB() = default;

inline Simd_AABB::Simd_AABB(AABB const& box) : min(Simdf(box.min().v)), max(Simdf(box.max().v)) {}

inline Simd_AABB::Simd_AABB(float const* min, float const* max)
    : min(Simdf::create_from_3(min)), max(Simdf::create_from_3(max)) {}

inline Simd_AABB::Simd_AABB(Simdf_p min, Simdf_p max) : min(min), max(max) {}

inline void Simd_AABB::merge_assign(Simd_AABB const& other) {
    min = math::min(min, other.min);
    max = math::max(max, other.max);
}

inline void Simd_AABB::merge_assign(Simdf_p other_min, Simdf_p other_max) {
    min = math::min(min, other_min);
    max = math::max(max, other_max);
}

}  // namespace math

#endif
