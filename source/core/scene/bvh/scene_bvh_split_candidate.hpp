#ifndef SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP
#define SU_CORE_SCENE_BVH_SPLIT_CANDIDATE_HPP

#include "base/math/aabb.hpp"
#include "base/math/plane.hpp"

#include <cstdint>
#include <vector>

namespace scene::bvh {



struct Reference {
    Reference() noexcept;

    uint32_t primitive() const noexcept;

    void set(float3 const& min, float3 const& max, uint32_t primitive) noexcept;

    void set(Simd3f const& min, Simd3f const& max, uint32_t primitive) noexcept;

    void clip_min(float d, uint8_t axis) noexcept;
    void clip_max(float d, uint8_t axis) noexcept;

    struct alignas(16) Vector {
        float    v[3];
        uint32_t index;
    };

    Vector bounds[2];
};

using References = std::vector<Reference>;

class Split_candidate {
  public:

    using index = typename std::vector<uint32_t>::const_iterator;

    Split_candidate(uint8_t split_axis, float3 const& pos, index begin, index end,
                    std::vector<AABB> const& aabbs) noexcept;

    bool behind(float3 const& point) const noexcept;

    uint8_t axis() const noexcept;

    float cost() const noexcept;

  private:
    float d_;

    uint8_t axis_;

    float cost_;
};

class Split_candidate1 {
      public:
        Split_candidate1(uint8_t split_axis, float3 const& p, bool spatial);

        void evaluate(References const& references, float aabb_surface_area);

        void distribute(References const& references, References& references0,
                        References& references1) const;

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
