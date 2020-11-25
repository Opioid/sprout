#ifndef SU_CORE_SCENE_LIGHT_TREE_BUILDER_HPP
#define SU_CORE_SCENE_LIGHT_TREE_BUILDER_HPP

#include "cstdint"

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
struct Light_pick;

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
    uint32_t split(Tree& tree, uint32_t node_id, uint32_t begin, uint32_t end, Scene const& scene,
                   Threads& threads);

    uint32_t split(Primitive_tree& tree, uint32_t node_id, uint32_t begin, uint32_t end,
                   uint32_t max_primitives, Part const& part, Threads& threads);

    void serialize(Node* nodes, uint32_t* node_middles);
    void serialize(Primitive_tree& tree, Part const& part);

    Build_node* build_nodes_;

    uint32_t current_node_;
    uint32_t light_order_;

    Split_candidate* candidates_;
};

}  // namespace light
}  // namespace scene

#endif
