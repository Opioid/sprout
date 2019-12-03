#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

#include "base/math/vector3.hpp"

#include <vector>

namespace scene {

class Scene;

namespace light {

class Light;

class Tree {
  public:
    struct Result {
        uint32_t id;
        float    pdf;
    };

    Result random_light(float3 const& p, float random) const noexcept;

    float pdf(float3 const& p, uint32_t id) const noexcept;

    struct Node {
        Node() noexcept;

        ~Node() noexcept;

        void gather() noexcept;

        float weight(float3 const& p) const noexcept;

        bool contains(uint32_t id) const noexcept;

        float3 center;

        float power;

        bool finite;

        uint32_t light;

        Node* children[2];
    };

    Node root_;
};

class Tree_builder {
  public:
    void build(Tree& tree, Scene const& scene) const noexcept;

  private:
    using Lights = std::vector<uint32_t>;

    void split(Tree::Node* node, uint32_t begin, uint32_t end, Lights const& lights,
               Scene const& scene) const noexcept;
};

}  // namespace light
}  // namespace scene

#endif
