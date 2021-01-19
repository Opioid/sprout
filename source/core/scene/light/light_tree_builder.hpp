#ifndef SU_CORE_SCENE_LIGHT_TREE_BUILDER_HPP
#define SU_CORE_SCENE_LIGHT_TREE_BUILDER_HPP

#include "base/math/vector.hpp"

namespace math {
struct AABB;
}

namespace thread {
class Pool;
}

using Threads = thread::Pool;

namespace scene {

class Scene;

namespace shape::triangle {
struct Part;
}

namespace light {

class Light;

struct Build_node;
struct Node;

class Tree;

class Primitive_tree;
struct Split_candidate;

class Tree_builder {
  public:
    using Part = shape::triangle::Part;

    Tree_builder();

    ~Tree_builder();

    void build(Tree& tree, Scene const& scene, Threads& threads);

    void build(Primitive_tree& tree, Part const& part, Threads& threads);

  private:
    void allocate(uint32_t num_lights, uint32_t sweep_threshold);

    uint32_t split(Tree& tree, uint32_t node_id, uint32_t begin, uint32_t end, AABB const& bounds,
                   float4_p cone, float total_power, Scene const& scene, Threads& threads);

    uint32_t split(Primitive_tree& tree, uint32_t node_id, uint32_t begin, uint32_t end,
                   uint32_t max_primitives, AABB const& bounds, float4_p cone, float total_power,
                   Part const& part, Threads& threads);

    uint32_t assign(Build_node& node, Primitive_tree& tree, uint32_t begin, uint32_t end,
                    AABB const& bounds, float4_p cone, float total_power, Part const& part);

    void serialize(Node* nodes, uint32_t* node_middles);
    void serialize(Primitive_tree& tree, Part const& part);

    uint32_t build_nodes_capacity_;
    uint32_t candidates_capacity_;

    uint32_t current_node_;
    uint32_t light_order_;

    Build_node* build_nodes_;

    Split_candidate* candidates_;
};

}  // namespace light
}  // namespace scene

#endif
