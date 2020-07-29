#include "scene_bvh_builder_base.hpp"

namespace scene::bvh {

inline void Build_node::allocate(uint8_t num_primitives) {
    max_.num_indices = num_primitives;
}

inline float3 Build_node::min() const {
    return float3(min_.v);
}

inline float3 Build_node::max() const {
    return float3(max_.v);
}

inline AABB Build_node::aabb() const {
    return {float3(min_.v), float3(max_.v)};
}

inline void Build_node::set_aabb(AABB const& aabb) {
    min_.v[0] = aabb.bounds[0][0];
    min_.v[1] = aabb.bounds[0][1];
    min_.v[2] = aabb.bounds[0][2];

    max_.v[0] = aabb.bounds[1][0];
    max_.v[1] = aabb.bounds[1][1];
    max_.v[2] = aabb.bounds[1][2];
}

inline uint8_t Build_node::num_indices() const {
    return max_.num_indices;
}

inline uint8_t Build_node::axis() const {
    return max_.axis;
}

}  // namespace scene::bvh
