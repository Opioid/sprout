#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP

#include "base/math/aabb.hpp"
#include "base/math/plane.hpp"

#include <cstdint>
#include <vector>

namespace scene::bvh {

class Split_candidate {
  public:
    Split_candidate(math::Plane const& plane, uint8_t axis) noexcept;

    Split_candidate(uint8_t split_axis, float3 const& pos, std::vector<uint32_t> const& indices,
                    std::vector<AABB> const& aabbs) noexcept;

    using index = typename std::vector<uint32_t>::const_iterator;

    Split_candidate(uint8_t split_axis, float3 const& pos, index begin, index end,
                    std::vector<AABB> const& aabbs) noexcept;

    uint64_t key() const noexcept;

    Plane const& plane() const noexcept;

    uint8_t axis() const noexcept;

  private:
    Plane plane_;

    uint64_t key_;

    uint8_t axis_;
};

}  // namespace scene::bvh

#endif
