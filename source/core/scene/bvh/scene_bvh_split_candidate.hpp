#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP

#include "base/math/aabb.hpp"
#include "base/memory/array.hpp"

#include <cstdint>
#include <vector>

namespace scene::bvh {

struct Reference {
    uint32_t primitive() const;

    void set(Simd3f_p min, Simd3f_p max, uint32_t primitive);

    Reference clipped_min(float d, uint8_t axis) const;
    Reference clipped_max(float d, uint8_t axis) const;

    struct alignas(16) Vector {
        float    v[3];
        uint32_t index;
    };

    Vector bounds[2];
};

using References = memory::Array<Reference>;

class Split_candidate {
  public:
    Split_candidate(uint8_t split_axis, float3_p p, bool spatial);

    void evaluate(References const& references, float aabb_surface_area);

    void distribute(References const& __restrict references, References& __restrict references0,
                    References& __restrict references1) const;

    float cost() const;

    bool behind(float const* point) const;

    uint8_t axis() const;

    bool spatial() const;

    AABB const& aabb_0() const;
    AABB const& aabb_1() const;

    uint32_t num_side_0() const;
    uint32_t num_side_1() const;

  private:
    AABB aabb_0_;
    AABB aabb_1_;

    uint32_t num_side_0_;
    uint32_t num_side_1_;

    float d_;

    float cost_;

    uint8_t axis_;

    bool spatial_;
};

}  // namespace scene::bvh

#endif
