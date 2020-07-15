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
  public:
    struct Build_node {
        Build_node();

        Build_node(Build_node&& other);

        ~Build_node();

        void allocate(uint8_t num_primitives);

        float3 min() const;

        float3 max() const;

        AABB aabb() const;

        void set_aabb(AABB const& aabb);

        uint32_t start_index() const;

        uint8_t num_indices() const;

        uint8_t axis() const;

        struct alignas(16) Min {
            float    v[3];
            uint32_t start_index;
        };

        struct alignas(16) Max {
            float   v[3];
            uint8_t axis;
            uint8_t num_indices;
            uint8_t pad[2];
        };

        Min min_;
        Max max_;

        uint32_t* primitives = nullptr;

        uint32_t children[2] = {0xFFFFFFFF, 0xFFFFFFFF};
    };

  protected:
    Builder_base(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives);

    virtual ~Builder_base();

    void split(uint32_t node_id, References& references, AABB const& aabb, uint32_t depth,
               thread::Pool& threads);

    Split_candidate splitting_plane(References const& references, AABB const& aabb, uint32_t depth,
                                    bool& exhausted, thread::Pool& threads);

    void assign(Build_node& node, References const& references);

    uint32_t const num_slices_;
    uint32_t const sweep_threshold_;
    uint32_t const max_primitives_;

    uint32_t num_references_;

    uint32_t spatial_split_threshold_;

    std::vector<Split_candidate> split_candidates_;

    std::vector<Build_node> build_nodes_;
};

}  // namespace scene::bvh

#endif
