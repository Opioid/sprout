#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

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

    float weight(float3 const& p) const noexcept;

    float3 center;

    float power;

    bool finite;

    uint32_t back;

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

    Result random_light(float3 const& p, float random) const noexcept;

    float pdf(float3 const& p, uint32_t id) const noexcept;

    Build_node root_;

    uint32_t* light_orders_;
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
