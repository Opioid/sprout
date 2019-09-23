#ifndef SU_CORE_SCENE_BVH_NODE_HPP
#define SU_CORE_SCENE_BVH_NODE_HPP

#include "base/math/vector3.hpp"
#include "base/simd/simd.hpp"

namespace math {
struct ray;
}

namespace scene::bvh {

class Node {
  public:
    Node() noexcept;

    float3 min() const noexcept;

    float3 max() const noexcept;

    uint32_t next() const noexcept;

    uint8_t axis() const noexcept;

    uint8_t num_primitives() const noexcept;

    uint32_t indices_start() const noexcept;

    uint32_t indices_end() const noexcept;

    void set_aabb(float const* min, float const* max) noexcept;

    void set_split_node(uint32_t next_node, uint8_t axis) noexcept;

    void set_leaf_node(uint32_t start_primitive, uint8_t num_primitives) noexcept;

    bool intersect_p(math::ray const& ray) const;

    bool intersect_p(Simd3f const& origin, Simd3f const& inv_direction, scalar const& min_t,
                     scalar const& max_t) const noexcept;

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
