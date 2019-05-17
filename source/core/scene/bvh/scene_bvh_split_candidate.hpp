#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP

#include <cstdint>
#include <vector>
#include "base/math/plane.hpp"

namespace scene::bvh {

template <typename T>
class Split_candidate {
  public:
    Split_candidate(math::Plane const& plane, uint8_t axis) noexcept;
    Split_candidate(uint8_t split_axis, float3 const& pos, std::vector<uint32_t> const& indices, std::vector<T*> const& props) noexcept;

    using index = typename std::vector<uint32_t>::iterator;

    Split_candidate(uint8_t split_axis, float3 const& pos, index begin, index end, std::vector<T*> const& props) noexcept;

    uint64_t key() const noexcept;

    math::Plane const& plane() const noexcept;

    uint8_t axis() const noexcept;

  private:
    math::Plane plane_;

    uint64_t key_;

    uint8_t axis_;
};

}  // namespace scene::bvh

#endif
