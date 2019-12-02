#ifndef SU_CORE_SCENE_LIGHT_TREE_HPP
#define SU_CORE_SCENE_LIGHT_TREE_HPP

#include "base/math/vector3.hpp"

namespace scene {

class Scene;

namespace light {

class Tree {
public:

    struct Result {
        float pdf;
        uint32_t id;
    };

    Result random_light(float3 const& p, float random) const noexcept;

    struct Node {
        float3 center;

        float power;

        Node* children[2];
    };

    Node root_;

};

class Tree_builder {
public:

    void build(Tree& tree, Scene const& scene) const noexcept;

};

}}

#endif
