#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL

#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "scene_bvh_split_candidate.hpp"

namespace scene::bvh {

uint32_t Reference::primitive() const {
    return bounds[0].index;
}

void Reference::set_min_max_primitive(float3 const& min, float3 const& max, uint32_t primitive) {
    bounds[0].v[0]  = min[0];
    bounds[0].v[1]  = min[1];
    bounds[0].v[2]  = min[2];
    bounds[0].index = primitive;

    bounds[1].v[0] = max[0];
    bounds[1].v[1] = max[1];
    bounds[1].v[2] = max[2];
    //   bounds[1].index = primitive;
}

void Reference::clip_min(float d, uint8_t axis) {
    bounds[0].v[axis] = std::max(d, bounds[0].v[axis]);
}

void Reference::clip_max(float d, uint8_t axis) {
    bounds[1].v[axis] = std::min(d, bounds[1].v[axis]);
}

Split_candidate::Split_candidate(uint8_t split_axis, float3 const& pos, index begin, index end,
                                 std::vector<AABB> const& aabbs, float aabb_surface_area) noexcept
    : axis_(split_axis), cost_(0.f) {
    d_ = pos[split_axis];

    AABB box_0 = AABB::empty();
    AABB box_1 = AABB::empty();

    int32_t num_side_0 = 0;
    int32_t num_side_1 = 0;

    uint32_t split = 0;

    for (index i = begin; i != end; ++i) {
        AABB const& b = aabbs[*i];

        bool const mib = behind(b.min());
        bool const mab = behind(b.max());

        if (mib && mab) {
            ++num_side_0;

            box_0.merge_assign(b);
        } else {
            if (mib != mab) {
                ++split;
            }

            ++num_side_1;

            box_1.merge_assign(b);
        }
    }

    float const total = float(std::distance(begin, end));

    cost_ += float(split) / total;

    //    cost_ += 0.0125f * float(std::abs(num_side_0 - num_side_1)) / total;

    if (0 == num_side_0) {
        cost_ += 1000.f;
    }

    //    if (bool const empty_side = 0 == num_side_0 || 0 == num_side_1; empty_side) {
    //        cost_ += 2.f + total;
    //    } else {
    //        float const weight_0 = float(num_side_0) * box_0.surface_area();
    //        float const weight_1 = float(num_side_1) * box_1.surface_area();

    //        cost_ += 2.f + (weight_0 + weight_1) / aabb_surface_area;
    //    }
}

bool Split_candidate::behind(float3 const& point) const noexcept {
    return point[axis_] < d_;
}

uint8_t Split_candidate::axis() const noexcept {
    return axis_;
}

float Split_candidate::cost() const noexcept {
    return cost_;
}

Split_candidate1::Split_candidate1(uint8_t split_axis, float3 const& p, bool spatial)
    : aabb_0_(AABB::empty()),
      aabb_1_(AABB::empty()),
      d_(p.v[split_axis]),
      axis_(split_axis),
      spatial_(spatial) {}

void Split_candidate1::evaluate(References const& references, float aabb_surface_area) {
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

void Split_candidate1::distribute(References const& references, References& references0,
                                  References& references1) const {
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

float Split_candidate1::cost() const {
    return cost_;
}

bool Split_candidate1::behind(float const* point) const {
    return point[axis_] < d_;
}

uint8_t Split_candidate1::axis() const {
    return axis_;
}

bool Split_candidate1::spatial() const {
    return spatial_;
}

AABB const& Split_candidate1::aabb_0() const {
    return aabb_0_;
}

AABB const& Split_candidate1::aabb_1() const {
    return aabb_1_;
}

uint32_t Split_candidate1::num_side_0() const {
    return num_side_0_;
}

uint32_t Split_candidate1::num_side_1() const {
    return num_side_1_;
}

}  // namespace scene::bvh

#endif
