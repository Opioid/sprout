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
    Node();

    float3 min() const;

    float3 max() const;

    uint32_t next() const;

    uint8_t axis() const;

    uint8_t num_primitives() const;

    uint32_t indices_start() const;

    uint32_t indices_end() const;

    void set_aabb(float const* min, float const* max);

    void set_split_node(uint32_t next_node, uint8_t axis);

    void set_leaf_node(uint32_t start_primitive, uint8_t num_primitives);

    bool intersect_p(Simd3f const& origin, Simd3f const& inv_direction, scalar const& min_t,
                     scalar const& max_t) const;

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
