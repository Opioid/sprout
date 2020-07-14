#ifndef SU_CORE_SCENE_BVH_BUILDER_BASE_HPP
#define SU_CORE_SCENE_BVH_BUILDER_BASE_HPP

#include "base/math/aabb.hpp"
#include "scene_bvh_split_candidate.hpp"

namespace thread {
class Pool;
}

namespace scene::bvh {

class Node;

class Builder_base {
  protected:
    Builder_base(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives);

    struct Build_node {
        Build_node();

        ~Build_node();

        void allocate(uint8_t num_primitives);

        AABB aabb;

        uint32_t start_index = 0;
        uint8_t  num_indices = 0;
        uint8_t  axis        = 0;

        uint32_t* primitives = nullptr;

        Build_node* children[2] = {nullptr, nullptr};
    };

    void split(Build_node* node, References& references, AABB const& aabb, uint32_t depth,
               thread::Pool& threads);

    Split_candidate splitting_plane(References const& references, AABB const& aabb, uint32_t depth,
                                    bool& exhausted, thread::Pool& threads);

    void assign(Build_node* node, References const& references);

    Node& new_node();

    uint32_t current_node_index() const;

    uint32_t const num_slices_;
    uint32_t const sweep_threshold_;
    uint32_t const max_primitives_;

    uint32_t current_node_;

    uint32_t num_nodes_;

    uint32_t num_references_;

    uint32_t spatial_split_threshold_;

    Node* nodes_;

    std::vector<Split_candidate> split_candidates_;
};

}  // namespace scene::bvh

#endif
