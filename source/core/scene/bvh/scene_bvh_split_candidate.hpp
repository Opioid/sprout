#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP

#include <cstdint>
#include <vector>
#include "base/math/plane.hpp"

namespace scene::bvh {

template <typename T>
class Split_candidate {
  public:
    Split_candidate(const math::Plane& plane, uint8_t axis);
    Split_candidate(uint8_t split_axis, float3 const& pos, const std::vector<T*>& data);

    using index = typename std::vector<T*>::iterator;

    Split_candidate(uint8_t split_axis, float3 const& pos, index begin, index end);

    uint64_t key() const;

    const math::Plane& plane() const;

    uint8_t axis() const;

  private:
    math::Plane plane_;

    uint64_t key_;

    uint8_t axis_;
};

}  // namespace scene::bvh

#endif
