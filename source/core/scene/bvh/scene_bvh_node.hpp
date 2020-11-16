#ifndef SU_CORE_SCENE_BVH_NODE_HPP
#define SU_CORE_SCENE_BVH_NODE_HPP

#include "base/math/vector3.hpp"
#include "base/simd/simd.hpp"

namespace math {
struct ray;
struct AABB;
}  // namespace math

namespace scene::bvh {

class alignas(32) Node {
  public:
    Node();

    Node(Node const& other, uint32_t offset);

    float3 min() const;

    float3 max() const;

    AABB aabb() const;

    uint32_t children() const;

    uint8_t axis() const;

    uint8_t num_indices() const;

    uint32_t indices_start() const;

    uint32_t indices_end() const;

    void set_aabb(float const* min, float const* max);

    void set_aabb(AABB const& aabb);

    void set_split_node(uint32_t children, uint8_t axis);

    void set_leaf_node(uint32_t start_primitive, uint8_t num_primitives);

    void offset(uint32_t offset);

    bool intersect_p(Simd3f_p origin, Simd3f_p inv_direction, scalar_p min_t, scalar_p max_t) const;

  private:
    struct alignas(16) Min {
        float    v[3];
        uint32_t children_or_data;
    };

    struct alignas(16) Max {
        float   v[3];
        uint8_t axis;
        uint8_t num_indices;
        uint8_t pad[2];
    };

    Min min_;
    Max max_;
};

}  // namespace scene::bvh

#endif
