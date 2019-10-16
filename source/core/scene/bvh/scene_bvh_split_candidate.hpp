#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP

#include "base/math/aabb.hpp"
#include "base/math/plane.hpp"

#include <cstdint>
#include <vector>

namespace scene::bvh {

class Split_candidate {
  public:

    using index = typename std::vector<uint32_t>::const_iterator;

    Split_candidate(uint8_t split_axis, float3 const& pos, index begin, index end,
                    std::vector<AABB> const& aabbs, float aabb_surface_area) noexcept;

    bool behind(float3 const& point) const noexcept;

    uint8_t axis() const noexcept;

    float cost() const noexcept;

  private:
    float d_;

    uint8_t axis_;

    float cost_;
};

}  // namespace scene::bvh

#endif
