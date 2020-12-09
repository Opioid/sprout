#ifndef SU_CORE_SCENE_BVH_BUILDER_BASE_HPP
#define SU_CORE_SCENE_BVH_BUILDER_BASE_HPP

#include "base/math/aabb.hpp"
#include "base/memory/array.hpp"

#include <atomic>
#include <vector>

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene::bvh {

class Node;
struct Reference;
class Split_candidate;

using References = memory::Array<Reference>;

class Kernel {
  public:
    Kernel(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives,
           uint32_t spatial_split_threshold);

    ~Kernel();

    struct Task {
        Task();

        Task(Kernel* k, uint32_t rt, uint32_t d, AABB const& box, References&& refs);

        Task(Task&& other);

        ~Task();

        Kernel* kernel = nullptr;

        uint32_t root;
        uint32_t depth;

        AABB aabb;

        References references;
    };

    using Tasks = std::vector<Task>;

    void split(uint32_t node_id, References& references, AABB const& aabb, uint32_t depth,
               Threads& threads, Tasks& tasks);

    Split_candidate splitting_plane(References const& references, AABB const& aabb, uint32_t depth,
                                    bool& exhausted, Threads& threads);

    void assign(Node& node, References const& references);

    void reserve(uint32_t num_primitives);

    uint32_t const num_slices_;
    uint32_t const sweep_threshold_;
    uint32_t const max_primitives_;

    uint32_t spatial_split_threshold_;

    uint32_t parallel_build_depth_;

    std::vector<uint32_t> reference_ids_;

    std::vector<Split_candidate> split_candidates_;

    std::vector<Node> build_nodes_;
};

class Builder_base : protected Kernel {
  protected:
    Builder_base(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives);

    ~Builder_base();

    void split(References& references, AABB const& aabb, Threads& threads);

    void work_on_tasks(Threads& threads, Tasks& tasks);

    void reserve(uint32_t num_primitives);

    void new_node();

    uint32_t current_node_index() const;

    uint32_t current_node_;

    Node* nodes_;

    std::atomic<uint32_t> current_task_;
};

}  // namespace scene::bvh

#endif
