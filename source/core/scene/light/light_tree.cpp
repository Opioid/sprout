#include "light_tree.hpp"
#include "base/spectrum/rgb.hpp"
#include "scene/scene.hpp"

namespace scene::light {

Tree::Node::Node() noexcept : children{nullptr, nullptr} {}

Tree::Node::~Node() {
    delete children[0];
    delete children[1];
}

Tree::Result Tree::random_light(float3 const& p, float random) const noexcept {
    return {0.f, 0xFFFFFFFF};
}

void Tree_builder::build(Tree& tree, Scene const& scene) const noexcept {
    delete tree.root_.children[0];
    delete tree.root_.children[1];

    tree.root_.children[0] = nullptr;
    tree.root_.children[1] = nullptr;

    Lights finite_lights;
    Lights infinite_lights;

    for (uint32_t l = 0, len = uint32_t(scene.lights().size()); l < len; ++l) {
        auto const& light = scene.lights()[l];

        if (light.is_finite(scene)) {
            finite_lights.push_back(l);
        } else {
            infinite_lights.push_back(l);
        }
    }

    if (infinite_lights.empty()) {
        split(&tree.root_, 0, uint32_t(finite_lights.size()), finite_lights, scene);
    } else {
        tree.root_.children[0] = new Tree::Node;
        tree.root_.children[1] = new Tree::Node;

        split(tree.root_.children[0], 0, uint32_t(finite_lights.size()), finite_lights, scene);
        split(tree.root_.children[1], 0, uint32_t(infinite_lights.size()), infinite_lights, scene);
    }
}

void Tree_builder::split(Tree::Node* node, uint32_t begin, uint32_t end, Lights const& lights,
                         Scene const& scene) const noexcept {
    if (1 == end - begin) {
        uint32_t const l = lights[begin];

        auto const& light = scene.lights()[l];

        node->center = scene.light_center(l);
        node->power  = spectrum::luminance(light.power(scene.aabb(), scene));
        node->finite = light.is_finite(scene);
    } else {
        node->children[0] = new Tree::Node;
        node->children[1] = new Tree::Node;

        if (scene.lights()[lights[begin]].is_finite(scene)) {
        } else {
            uint32_t const middle = (end - begin) / 2;

            split(node->children[0], begin, middle, lights, scene);
            split(node->children[1], middle, end, lights, scene);
        }
    }
}

}  // namespace scene::light
