#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "base/math/vector3.hpp"

#include <vector>

namespace scene {

class Scene;

namespace light {

class Light;

struct alignas(32) Build_node {
    void gather(uint32_t const* orders, Build_node* nodes);

    float3 center;

    float power;

    uint32_t middle;
    uint32_t end;

    uint32_t children_or_light;
};

class Tree {
  public:
    Tree();

    ~Tree();

    struct Result {
        uint32_t id;
        float    pdf;
    };

    struct alignas(32) Node {
        float weight(float3 const& p, float3 const& n, bool total_sphere) const;

        float3 center;

        float power;

        bool children;

        uint32_t middle;

        uint32_t children_or_light;
    };

    Result random_light(float3 const& p, float3 const& n, bool total_sphere, float random) const;

    float pdf(float3 const& p, float3 const& n, bool total_sphere, uint32_t id) const;

    void allocate(uint32_t num_finite_lights, uint32_t num_infinite_lights);

    float infinite_weight_;
    float infinite_guard_;

    uint32_t infinite_end_;

    uint32_t num_finite_lights_;
    uint32_t num_infinite_lights_;

    Node* nodes_;

    uint32_t* light_orders_;

    float* infinite_light_powers_;

    Distribution_implicit_pdf_lut_lin_1D infinite_light_distribution_;
};

class Tree_builder {
  public:
    Tree_builder();

    ~Tree_builder();

    void build(Tree& tree, Scene const& scene);

  private:
    using Lights = std::vector<uint32_t>;

    void split(Tree& tree, uint32_t node_id, uint32_t begin, uint32_t end, Lights& lights,
               Scene const& scene);

    void serialize(uint32_t num_nodes);

    Build_node* build_nodes_;

    Tree::Node* nodes_;

    uint32_t current_node_;

    uint32_t light_order_;
};

}  // namespace light
}  // namespace scene

#endif
