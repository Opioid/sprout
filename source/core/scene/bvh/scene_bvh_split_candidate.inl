#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_INL

#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "scene_bvh_split_candidate.hpp"

namespace scene::bvh {

template <typename T>
Split_candidate<T>::Split_candidate(const math::Plane& plane, uint8_t axis)
    : plane_(plane), axis_(axis) {}

template <typename T>
Split_candidate<T>::Split_candidate(uint8_t split_axis, float3 const& pos,
                                    const std::vector<T*>& data)
    : Split_candidate(split_axis, pos, data.begin(), data.end()) {}

template <typename T>
Split_candidate<T>::Split_candidate(uint8_t split_axis, float3 const& pos, index begin, index end)
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

    plane_ = math::plane::create(n, pos);

    int      num_side_0 = 0;
    int      num_side_1 = 0;
    uint32_t split      = 0;

    for (index i = begin; i != end; ++i) {
        bool mib = math::plane::behind(plane_, (*i)->aabb().min());
        bool mab = math::plane::behind(plane_, (*i)->aabb().max());

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

template <typename T>
uint64_t Split_candidate<T>::key() const {
    return key_;
}

template <typename T>
const math::Plane& Split_candidate<T>::plane() const {
    return plane_;
}

template <typename T>
uint8_t Split_candidate<T>::axis() const {
    return axis_;
}

}  // namespace scene::bvh

#endif
