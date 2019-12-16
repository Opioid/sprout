#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "base/math/vector3.hpp"

#include <vector>

namespace scene {

class Scene;

namespace light {

class Light;

struct Build_node {
    Build_node() noexcept;

    ~Build_node() noexcept;

    void gather(uint32_t const* orders) noexcept;

    float weight(float3 const& p, float3 const& n, bool total_sphere) const noexcept;

    float3 center;

    float power;

    uint32_t middle;
    uint32_t end;

    uint32_t light;

    Build_node* children[2];
};

class Tree {
  public:
    Tree() noexcept;

    ~Tree() noexcept;

    struct Result {
        uint32_t id;
        float    pdf;
    };

    struct Node {
        float weight(float3 const& p, float3 const& n, bool total_sphere) const noexcept;

        float3 center;

        float power;

        bool children;

        uint32_t middle;

        uint32_t next_or_light;
    };

    Result random_light(float3 const& p, float3 const& n, bool total_sphere, float random) const
        noexcept;

    float pdf(float3 const& p, float3 const& n, bool total_sphere, uint32_t id) const noexcept;

    void allocate(uint32_t num_finite_lights, uint32_t num_infinite_lights) noexcept;

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
    void build(Tree& tree, Scene const& scene) noexcept;

  private:
    using Lights = std::vector<uint32_t>;

    void split(Tree& tree, Build_node* node, uint32_t begin, uint32_t end, Lights& lights,
               Scene const& scene) noexcept;

    void serialize(Build_node* node) noexcept;

    Tree::Node* nodes_;

    uint32_t current_;

    uint32_t light_order_;
};

}  // namespace light
}  // namespace scene

#endif
