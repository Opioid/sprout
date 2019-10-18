#ifndef SU_CORE_SCENE_BVH_BUILDER_BASE_HPP
#define SU_CORE_SCENE_BVH_BUILDER_BASE_HPP

#include "base/math/aabb.hpp"
#include "scene_bvh_split_candidate.hpp"

#include <cstdint>
#include <vector>

namespace thread {
class Pool;
}

namespace scene::bvh {

class Node;

class Builder_base {
  protected:
    Builder_base(uint32_t num_slices, uint32_t sweep_threshold) noexcept;

    struct Build_node {
        Build_node() noexcept;

        ~Build_node() noexcept;

        AABB aabb;

        uint8_t axis;

        std::vector<uint32_t> primitives;

        uint32_t start_index = 0;
        uint32_t end_index   = 0;

        Build_node* children[2] = {nullptr, nullptr};
    };

    void split(Build_node* node, References& references, AABB const& aabb, uint32_t max_primitives,
               uint32_t depth, thread::Pool& pool) noexcept;

    Split_candidate splitting_plane(References const& references, AABB const& aabb, uint32_t depth,
                                    bool& exhausted, thread::Pool& pool) noexcept;

    void assign(Build_node* node, References const& references) noexcept;

    Node& new_node() noexcept;

    uint32_t current_node_index() const noexcept;

    uint32_t current_node_;

    uint32_t num_nodes_;

    uint32_t num_references_;

    uint32_t spatial_split_threshold_;

    uint32_t const num_slices_;
    uint32_t const sweep_threshold_;

    Node* nodes_;

    std::vector<Split_candidate> split_candidates_;
};

}  // namespace scene::bvh

#endif
