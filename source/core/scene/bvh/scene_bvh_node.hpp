#ifndef SU_CORE_SCENE_BVH_NODE_HPP
#define SU_CORE_SCENE_BVH_NODE_HPP

#include "base/math/vector3.hpp"
//#include "base/math/ray.hpp"
#include "base/simd/simd.hpp"

namespace scene::bvh {

class Node {
  public:
    Node() noexcept = default;

    float3 min() const noexcept {
        return float3(min_.v);
    }

    float3 max() const noexcept {
        return float3(max_.v);
    }

    uint32_t next() const noexcept {
        return min_.next_or_data;
    }

    uint8_t axis() const noexcept {
        return max_.axis;
    }

    uint8_t num_primitives() const noexcept {
        return max_.num_primitives;
    }

    uint32_t indices_start() const noexcept {
        return min_.next_or_data;
    }

    uint32_t indices_end() const noexcept {
        return min_.next_or_data + static_cast<uint32_t>(max_.num_primitives);
    }

    void set_aabb(float const* min, float const* max) noexcept;

    void set_split_node(uint32_t next_node, uint8_t axis) noexcept;

    void set_leaf_node(uint32_t start_primitive, uint8_t num_primitives) noexcept;

    //	bool intersect_p(ray const& ray) const;

    bool intersect_p(FVector origin, FVector inv_direction, FVector min_t, FVector max_t) const
        noexcept;

  private:
    struct alignas(16) Min {
        float    v[3];
        uint32_t next_or_data;
    };

    struct alignas(16) Max {
        float   v[3];
        uint8_t axis;
        uint8_t num_primitives;
        uint8_t pad[2];
    };

    Min min_;
    Max max_;
};

}  // namespace scene::bvh

#endif
