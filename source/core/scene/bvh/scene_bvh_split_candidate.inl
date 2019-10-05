#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL

#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "scene_bvh_split_candidate.hpp"

namespace scene::bvh {

Split_candidate::Split_candidate(Plane const& plane, uint8_t axis) noexcept
    : plane_(plane), axis_(axis) {}

Split_candidate::Split_candidate(uint8_t split_axis, float3 const& pos,
                                 std::vector<uint32_t> const& indices,
                                 std::vector<AABB> const&     aabbs) noexcept
    : Split_candidate(split_axis, pos, indices.begin(), indices.end(), aabbs) {}

Split_candidate::Split_candidate(uint8_t split_axis, float3 const& pos, index begin, index end,
                                 std::vector<AABB> const& aabbs) noexcept
    : axis_(split_axis) {
    key_ = 0;

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

    plane_ = plane::create(n, pos);

    int32_t num_side_0 = 0;
    int32_t num_side_1 = 0;

    uint32_t split = 0;

    for (index i = begin; i != end; ++i) {
        AABB const& b = aabbs[*i];

        bool const mib = plane::behind(plane_, b.min());
        bool const mab = plane::behind(plane_, b.max());

        if (mib && mab) {
            ++num_side_0;
        } else {
            if (mib != mab) {
                ++split;
            }

            ++num_side_1;
        }
    }

    key_ += split;

    if (0 == num_side_0) {
        key_ += 0x1000000000000000;
    }
}

uint64_t Split_candidate::key() const noexcept {
    return key_;
}

Plane const& Split_candidate::plane() const noexcept {
    return plane_;
}

uint8_t Split_candidate::axis() const noexcept {
    return axis_;
}

}  // namespace scene::bvh

#endif
