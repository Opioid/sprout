#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

#include "base/math/distribution/distribution_1d.hpp"
#include "base/math/vector3.hpp"
#include "base/memory/array.hpp"

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

    Result random_light(float3 const& p, float3 const& n, bool total_sphere, float random) const
        noexcept;

    float pdf(float3 const& p, float3 const& n, bool total_sphere, uint32_t id) const noexcept;

    memory::Array<uint32_t> light_orders_;

    float infinite_weight_;
    float infinite_guard_;

    uint32_t infinite_end_;

    Build_node root_;

    memory::Array<float> infinite_light_powers_;

    Distribution_implicit_pdf_lut_lin_1D infinite_light_distribution_;
};

class Tree_builder {
  public:
    void build(Tree& tree, Scene const& scene) noexcept;

  private:
    using Lights = std::vector<uint32_t>;

    void split(Tree& tree, Build_node* node, uint32_t begin, uint32_t end, Lights const& lights,
               Scene const& scene) noexcept;

    uint32_t light_order_;
};

}  // namespace light
}  // namespace scene

#endif
