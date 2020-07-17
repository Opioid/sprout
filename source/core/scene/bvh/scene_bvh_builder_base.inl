#include "scene_bvh_builder_base.hpp"

namespace scene::bvh {

inline Builder_base::Build_node::Build_node() = default;

inline Builder_base::Build_node::Build_node(Build_node&& other)
    : min_(other.min_),
      max_(other.max_),
      primitives(other.primitives),
      children{other.children[0], other.children[1]} {
    other.primitives = nullptr;
}

inline Builder_base::Build_node::~Build_node() {
    delete[] primitives;
}

inline void Builder_base::Build_node::allocate(uint8_t num_primitives) {
    max_.num_indices = num_primitives;

    primitives = new uint32_t[num_primitives];
}

inline float3 Builder_base::Build_node::min() const {
    return float3(min_.v);
}

inline float3 Builder_base::Build_node::max() const {
    return float3(max_.v);
}

inline AABB Builder_base::Build_node::aabb() const {
    return {float3(min_.v), float3(max_.v)};
}

inline void Builder_base::Build_node::set_aabb(AABB const& aabb) {
    min_.v[0] = aabb.bounds[0][0];
    min_.v[1] = aabb.bounds[0][1];
    min_.v[2] = aabb.bounds[0][2];

    max_.v[0] = aabb.bounds[1][0];
    max_.v[1] = aabb.bounds[1][1];
    max_.v[2] = aabb.bounds[1][2];
}

inline uint32_t Builder_base::Build_node::start_index() const {
    return min_.start_index;
}

inline uint8_t Builder_base::Build_node::num_indices() const {
    return max_.num_indices;
}

inline uint8_t Builder_base::Build_node::axis() const {
    return max_.axis;
}

}  // namespace scene::bvh
