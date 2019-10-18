#ifndef SU_CORE_SCENE_BVH_BUILDER_HPP
#define SU_CORE_SCENE_BVH_BUILDER_HPP

#include "scene_bvh_builder_base.hpp"
#include "base/math/aabb.hpp"
#include "scene_bvh_split_candidate.hpp"

#include <cstddef>
#include <vector>

namespace thread {
class Pool;
}

namespace scene::bvh {

class Node;
struct Tree;

class Builder : private Builder_base {
  public:
    Builder() noexcept;

    ~Builder() noexcept;

    void build(Tree& tree, std::vector<uint32_t>& indices, std::vector<AABB> const& aabbs, thread::Pool& pool) noexcept;

  private:


    using const_index = typename std::vector<uint32_t>::const_iterator;
    using index       = typename std::vector<uint32_t>::iterator;

    void split(Build_node* node, References& references, AABB const& aabb, uint32_t max_primitives,
               uint32_t depth, thread::Pool& pool);

    Split_candidate splitting_plane(References const& references, AABB const& aabb, uint32_t depth,
                                    bool& exhausted, thread::Pool& pool);

    void serialize(Build_node* node, Tree& tree, uint32_t& current_prop) noexcept;

    Node& new_node() noexcept;

    uint32_t current_node_index() const noexcept;

    void assign(Build_node* node, References const& references);

    std::vector<Split_candidate> split_candidates_;

    Build_node* root_;

    uint32_t num_nodes_;
    uint32_t current_node_;

    Node* nodes_;

     uint32_t num_references_;

     uint32_t spatial_split_threshold_;

     uint32_t const num_slices_;
     uint32_t const sweep_threshold_;
};

}  // namespace scene::bvh

#endif
