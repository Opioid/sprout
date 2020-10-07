#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

#include "base/math/aabb.hpp"
#include "base/math/distribution/distribution_1d.hpp"
#include "base/math/vector4.hpp"
#include "base/memory/array.hpp"
#include "light.hpp"

#include <vector>

namespace scene {

class Scene;

namespace light {

class Light;

struct Build_node {
    AABB bounds;

    float4 cone;

    float power;

    uint32_t middle;
    uint32_t children_or_light;
    uint32_t num_lights;
};

class Tree {
  public:
    using Lights = memory::Array<Light_ptr>;

    Tree();

    ~Tree();

    struct Result {
        uint32_t id;
        float    pdf;
    };

    struct Node {
        float weight(float3 const& p, float3 const& n, bool total_sphere) const;

        float weight(float3 const& v0, float3 const& v1, float3 const& dir) const;

        Result random_light(float3 const& p, float3 const& n, bool total_sphere, float random,
                            uint32_t const* const light_mapping, Scene const& scene) const;

        Result random_light(float3 const& p0, float3 const& p1, float3 const& dir, float random,
                            uint32_t const* const light_mapping, Scene const& scene) const;

        float pdf(float3 const& p, float3 const& n, bool total_sphere, uint32_t id,
                  uint32_t const* const light_mapping, Scene const& scene) const;

        float4 center;
        float4 cone;

        float power;

        uint32_t middle;
        uint32_t children_or_light;
        uint32_t num_lights;
    };

    Result random_light(float3 const& p, float3 const& n, bool total_sphere, float random,
                        Scene const& scene) const;

    void random_light(float3 const& p, float3 const& n, bool total_sphere, float random,
                      Scene const& scene, Lights& lights) const;

    Result random_light(float3 const& p0, float3 const& p1, float random, Scene const& scene) const;

    float pdf(float3 const& p, float3 const& n, bool total_sphere, uint32_t id,
              Scene const& scene) const;

    void allocate_light_mapping(uint32_t num_lights);

    void allocate(uint32_t num_infinite_lights);

    void allocate_nodes(uint32_t num_nodes);

    float infinite_weight_;
    float infinite_guard_;

    uint32_t infinite_end_;
    uint32_t num_lights_;
    uint32_t num_infinite_lights_;
    uint32_t num_nodes_;

    Node* nodes_;

    uint32_t* light_orders_;
    uint32_t* light_mapping_;

    float* infinite_light_powers_;

    Distribution_1D infinite_light_distribution_;
};

class Tree_builder {
  public:
    Tree_builder();

    ~Tree_builder();

    void build(Tree& tree, Scene const& scene);

    struct Split_candidate {
        Split_candidate();

        void init(uint32_t begin, uint32_t end, uint32_t split, float surface_area,
                  float cone_weight, uint32_t const* const lights, Scene const& scene);

        uint32_t split_node;

        float weight;
    };

  private:
    uint32_t split(Tree& tree, uint32_t node_id, uint32_t begin, uint32_t end, Scene const& scene);

    void serialize(Tree::Node* nodes);

    Build_node* build_nodes_;

    uint32_t current_node_;

    uint32_t light_order_;

    Split_candidate* candidates_;
};

}  // namespace light
}  // namespace scene

#endif
