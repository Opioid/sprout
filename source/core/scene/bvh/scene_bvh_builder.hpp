#ifndef SU_CORE_SCENE_BVH_BUILDER_HPP
#define SU_CORE_SCENE_BVH_BUILDER_HPP

#include <cstddef>
#include <vector>
#include "base/math/aabb.hpp"
#include "base/math/plane.hpp"
#include "scene_bvh_split_candidate.hpp"

namespace scene::bvh {

class Node;
struct Tree;

class Builder {
  public:
    Builder() noexcept;

    ~Builder() noexcept;

    void build(Tree& tree, std::vector<uint32_t>& indices, std::vector<AABB> const& aabbs) noexcept;

  private:
    struct Build_node {
        Build_node() noexcept = default;

        ~Build_node() noexcept;

        void clear() noexcept;

        AABB aabb;

        uint8_t axis;

        uint32_t offset;
        uint32_t props_end;

        Build_node* children[2] = {nullptr, nullptr};
    };

    using const_index = typename std::vector<uint32_t>::const_iterator;
    using index       = typename std::vector<uint32_t>::iterator;

    void split(Build_node* node, index begin, index end, const_index origin,
               std::vector<AABB> const& aabbs, uint32_t max_shapes) noexcept;

    Split_candidate splitting_plane(AABB const& aabb, index begin, index end,
                                    std::vector<AABB> const& aabbs) noexcept;

    void serialize(Build_node* node) noexcept;

    Node& new_node() noexcept;

    uint32_t current_node_index() const noexcept;

    static void assign(Build_node* node, const_index begin, const_index end,
                       const_index origin) noexcept;

    static AABB aabb(index begin, index end, std::vector<AABB> const& aabbs) noexcept;

    std::vector<Split_candidate> split_candidates_;

    Build_node* root_;

    uint32_t num_nodes_;
    uint32_t current_node_;

    Node* nodes_;
};

}  // namespace scene::bvh

#endif
