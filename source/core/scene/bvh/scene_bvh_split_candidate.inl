#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL

#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "scene_bvh_split_candidate.hpp"

namespace scene::bvh {

inline Reference::Reference() noexcept = default;

inline uint32_t Reference::primitive() const noexcept {
    return bounds[0].index;
}

inline void Reference::set(Simd3f const& min, Simd3f const& max, uint32_t primitive) noexcept {
    float3 const tmp(min);
    bounds[0].v[0]  = tmp[0];
    bounds[0].v[1]  = tmp[1];
    bounds[0].v[2]  = tmp[2];
    bounds[0].index = primitive;

    simd::store_float4(bounds[1].v, max.v);
}

inline void Reference::clip_min(float d, uint8_t axis) noexcept {
    bounds[0].v[axis] = std::max(d, bounds[0].v[axis]);
}

inline void Reference::clip_max(float d, uint8_t axis) noexcept {
    bounds[1].v[axis] = std::min(d, bounds[1].v[axis]);
}

inline Split_candidate::Split_candidate(uint8_t split_axis, float3 const& p, bool spatial) noexcept
    : aabb_0_(AABB::empty()),
      aabb_1_(AABB::empty()),
      d_(p.v[split_axis]),
      axis_(split_axis),
      spatial_(spatial) {}

inline void Split_candidate::evaluate(References const& references,
                                      float             aabb_surface_area) noexcept {
    uint32_t num_side_0 = 0;
    uint32_t num_side_1 = 0;

    Simd_AABB box_0(aabb_0_);
    Simd_AABB box_1(aabb_1_);

    if (spatial_) {
        for (auto const& r : references) {
            Simd_AABB b(r.bounds[0].v, r.bounds[1].v);

            if (behind(r.bounds[1].v)) {
                ++num_side_0;

                box_0.merge_assign(b);
            } else if (!behind(r.bounds[0].v)) {
                ++num_side_1;

                box_1.merge_assign(b);
            } else {
                ++num_side_0;
                ++num_side_1;

                box_0.merge_assign(b);
                box_1.merge_assign(b);
            }
        }

        aabb_0_.set_min_max(box_0.min, box_0.max);
        aabb_1_.set_min_max(box_1.min, box_1.max);

        aabb_0_.clip_max(d_, axis_);
        aabb_1_.clip_min(d_, axis_);
    } else {
        for (auto const& r : references) {
            Simd_AABB b(r.bounds[0].v, r.bounds[1].v);

            if (behind(r.bounds[1].v)) {
                ++num_side_0;

                box_0.merge_assign(b);
            } else {
                ++num_side_1;

                box_1.merge_assign(b);
            }
        }

        aabb_0_.set_min_max(box_0.min, box_0.max);
        aabb_1_.set_min_max(box_1.min, box_1.max);
    }

    if (bool const empty_side = 0 == num_side_0 || 0 == num_side_1; empty_side) {
        cost_ = 2.f + float(references.size());
    } else {
        float const weight_0 = float(num_side_0) * aabb_0_.surface_area();
        float const weight_1 = float(num_side_1) * aabb_1_.surface_area();

        cost_ = 2.f + (weight_0 + weight_1) / aabb_surface_area;
    }

    num_side_0_ = num_side_0;
    num_side_1_ = num_side_1;
}

inline void Split_candidate::distribute(References const& references, References& references0,
                                        References& references1) const noexcept {
    references0.reserve(num_side_0_);
    references1.reserve(num_side_1_);

    if (spatial_) {
        for (auto const& r : references) {
            if (behind(r.bounds[1].v)) {
                references0.push_back(r);
            } else if (!behind(r.bounds[0].v)) {
                references1.push_back(r);
            } else {
                Reference r0 = r;
                r0.clip_max(d_, axis_);
                references0.push_back(r0);

                Reference r1 = r;
                r1.clip_min(d_, axis_);
                references1.push_back(r1);
            }
        }
    } else {
        for (auto const& r : references) {
            if (behind(r.bounds[1].v)) {
                references0.push_back(r);
            } else {
                references1.push_back(r);
            }
        }
    }
}

inline float Split_candidate::cost() const noexcept {
    return cost_;
}

inline bool Split_candidate::behind(float const* point) const noexcept {
    return point[axis_] < d_;
}

inline uint8_t Split_candidate::axis() const noexcept {
    return axis_;
}

inline bool Split_candidate::spatial() const noexcept {
    return spatial_;
}

inline AABB const& Split_candidate::aabb_0() const noexcept {
    return aabb_0_;
}

inline AABB const& Split_candidate::aabb_1() const noexcept {
    return aabb_1_;
}

inline uint32_t Split_candidate::num_side_0() const noexcept {
    return num_side_0_;
}

inline uint32_t Split_candidate::num_side_1() const noexcept {
    return num_side_1_;
}

}  // namespace scene::bvh

#endif
