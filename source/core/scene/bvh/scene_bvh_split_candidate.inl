#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL

#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "scene_bvh_split_candidate.hpp"

namespace scene::bvh {

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

}  // namespace scene::bvh

#endif
