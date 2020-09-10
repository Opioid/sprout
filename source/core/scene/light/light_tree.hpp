#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "base/math/vector4.hpp"
#include "base/math/aabb.hpp"

#include <vector>

namespace scene {

class Scene;

namespace light {

class Light;

struct alignas(32) Build_node {
    void gather(Build_node* nodes);

    float3 center;

    float power;

    float4 cone;

    uint32_t middle;
    uint32_t end;

    uint32_t children_or_light;

    uint32_t num_lights;

    AABB box;
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

        static float light_weight(float3 const& p, float3 const& n, bool total_sphere, uint32_t light, Scene const& scene);

        Result random_light(float3 const& p, float3 const& n, bool total_sphere, float random, uint32_t const* const light_mapping, Scene const& scene) const;

        float pdf(float3 const& p, float3 const& n, bool total_sphere, uint32_t id, uint32_t const* const light_mapping, Scene const& scene) const;

        float3 center;

        float4 cone;

        float radius;

        float power;

        uint32_t middle;

        uint32_t children_or_light;

        uint32_t num_lights;
    };

    Result random_light(float3 const& p, float3 const& n, bool total_sphere, float random, Scene const& scene) const;

    float pdf(float3 const& p, float3 const& n, bool total_sphere, uint32_t id, Scene const& scene) const;

    void allocate(uint32_t num_finite_lights, uint32_t num_infinite_lights);

    float infinite_weight_;
    float infinite_guard_;

    uint32_t infinite_end_;

    uint32_t num_finite_lights_;
    uint32_t num_infinite_lights_;

    Node* nodes_;

    uint32_t* light_orders_;

    float* infinite_light_powers_;

    Distribution_1D infinite_light_distribution_;

    std::vector<uint32_t> light_mapping_;
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

    struct Split_candidate {
        Split_candidate();

        void init(uint32_t begin, uint32_t end, uint32_t split, float total_power, float aabb_surface_area, Lights const& lights, Scene const& scene);

        uint32_t split_node;

        float weight;
    };

    Split_candidate* candidates_;
};

}  // namespace light
}  // namespace scene

#endif
