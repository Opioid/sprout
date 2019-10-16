#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL

#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "scene_bvh_split_candidate.hpp"

namespace scene::bvh {

Split_candidate::Split_candidate(uint8_t split_axis, float3 const& pos,
                                 std::vector<uint32_t> const& indices,
                                 std::vector<AABB> const&     aabbs) noexcept
    : Split_candidate(split_axis, pos, indices.begin(), indices.end(), aabbs) {}

Split_candidate::Split_candidate(uint8_t split_axis, float3 const& pos, index begin, index end,
                                 std::vector<AABB> const& aabbs) noexcept
    : axis_(split_axis), cost_(0.f) {

    float3 n;

    switch (split_axis) {
        default:
        case 0:
            n = float3(1.f, 0.f, 0.f);
            break;
        case 1:
            n = float3(0.f, 1.f, 0.f);
            break;
        case 2:
            n = float3(0.f, 0.f, 1.f);
            break;
    }

    d_ = pos[split_axis];

    int32_t num_side_0 = 0;
    int32_t num_side_1 = 0;

    uint32_t split = 0;

    for (index i = begin; i != end; ++i) {
        AABB const& b = aabbs[*i];

        bool const mib = behind(b.min());
        bool const mab = behind(b.max());

        if (mib && mab) {
            ++num_side_0;
        } else {
            if (mib != mab) {
                ++split;
            }

            ++num_side_1;
        }
    }

    float const total = float(std::distance(begin, end));

    cost_ += float(split) / total;

//    cost_ += 0.0125f * float(std::abs(num_side_0 - num_side_1)) / total;

    if (0 == num_side_0) {
        cost_ += 1000.f;
    }
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
