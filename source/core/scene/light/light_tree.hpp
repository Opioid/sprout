#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "base/math/vector.hpp"
#include "base/memory/array.hpp"

namespace math {
struct AABB;
}

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

class Tree {
  public:
    static uint32_t constexpr Max_split_depth = 4;

    // (Max_split_depth + 1) to have space for worst case where we want to split on leaf node,
    // which can have up to 4 lights
    static uint32_t constexpr Max_lights = 1 << (Max_split_depth + 1);

    static uint32_t max_lights(uint32_t num_lights, bool split);

    static void set_splitting_threshold(float st);

    static float splitting_threshold_;

    using Lights = memory::Array<Light_pick>;

    Tree();

    ~Tree();

    void random_light(float3_p p, float3_p n, bool total_sphere, float random, bool split,
                      Scene const& scene, Lights& lights) const;

    void random_light(float3_p p0, float3_p p1, float random, bool split, Scene const& scene,
                      Lights& lights) const;

    float pdf(float3_p p, float3_p n, bool total_sphere, bool split, uint32_t id,
              Scene const& scene) const;

    void allocate_light_mapping(uint32_t num_lights);

    void allocate_nodes(uint32_t num_nodes);

    void allocate(uint32_t num_infinite_lights);

    float infinite_weight_;
    float infinite_guard_;

    uint32_t infinite_end_;
    uint32_t infinite_depth_bias_;
    uint32_t num_lights_;
    uint32_t num_infinite_lights_;
    uint32_t num_nodes_;

    Node*     nodes_;
    uint32_t* node_middles_;

    uint32_t* light_orders_;
    uint32_t* light_mapping_;

    float* infinite_light_powers_;

    Distribution_1D infinite_light_distribution_;
};

class Primitive_tree {
public:

    using Part = shape::triangle::Part;

    Primitive_tree();

    ~Primitive_tree();

    Light_pick random_light(float3_p p, float3_p n, bool total_sphere, float random,
                      Part const& part) const;

    void allocate_light_mapping(uint32_t num_lights);

    void allocate_nodes(uint32_t num_nodes);


    uint32_t num_lights_;
    uint32_t num_nodes_;

    Node*     nodes_;
    uint32_t* node_middles_;

    uint32_t* light_orders_;
    uint32_t* light_mapping_;
};

using UInts = uint32_t const* const;

class Tree_builder {
  public:
    using Part = shape::triangle::Part;

    Tree_builder();

    ~Tree_builder();

    void build(Tree& tree, Scene const& scene);

    void build(Primitive_tree& tree, Part const& part);

    struct Split_candidate {
        Split_candidate();

        template <typename Set>
        void init(uint32_t begin, uint32_t end, uint32_t split, float surface_area,
                  float cone_weight, UInts lights, Set const& set);

        uint32_t split_node;

        float weight;
    };

  private:
    uint32_t split(Tree& tree, uint32_t node_id, uint32_t begin, uint32_t end, Scene const& scene);

    uint32_t split(Primitive_tree& tree, uint32_t node_id, uint32_t begin, uint32_t end, Part const& part);

    void serialize(Node* nodes, uint32_t* node_middles);

    Build_node* build_nodes_;

    uint32_t current_node_;
    uint32_t light_order_;

    Split_candidate* candidates_;
};

}  // namespace light
}  // namespace scene

#endif
