#include "light_tree.hpp"
#include "base/spectrum/rgb.hpp"
#include "scene/scene.inl"

namespace scene::light {

Tree::Node::Node() noexcept : children{nullptr, nullptr} {}

Tree::Node::~Node() {
    delete children[0];
    delete children[1];
}

void Tree::Node::gather() noexcept {
    if (children[0]) {
        children[0]->gather();
        children[1]->gather();

        float const total_power = children[0]->power + children[1]->power;

        center = (children[0]->power * children[0]->center +
                  children[1]->power * children[1]->center) /
                 total_power;

        power = total_power;
    }
}

float Tree::Node::weight(float3 const& p) const noexcept {
    if (finite) {
        return power / squared_distance(center, p);
    } else {
        return power;
    }
}

bool Tree::Node::contains(uint32_t id) const noexcept {
    if (!children[0]) {
        return light == id;
    }

    return children[0]->contains(id) || children[1]->contains(id);
}

Tree::Result Tree::random_light(float3 const& p, float random) const noexcept {
    Node const* node = &root_;

    float pdf = 1.f;

    for (;;) {
        if (node->children[0]) {
            float p0 = node->children[0]->weight(p);
            float p1 = node->children[1]->weight(p);

            float const pt = p0 + p1;

            p0 /= pt;
            p1 /= pt;

            if (random < p0) {
                node = node->children[0];
                pdf *= p0;
                random /= p0;
            } else {
                node = node->children[1];
                pdf *= p1;
                random = (random - p0) / p1;
            }
        } else {
            return {node->light, pdf};
        }
    }
}

float Tree::pdf(float3 const& p, uint32_t id) const noexcept {
    Node const* node = &root_;

    float pdf = 1.f;

    for (;;) {
        if (node->children[0]) {
            float p0 = node->children[0]->weight(p);
            float p1 = node->children[1]->weight(p);

            float const pt = p0 + p1;

            p0 /= pt;
            p1 /= pt;

            if (node->children[0]->contains(id)) {
                node = node->children[0];
                pdf *= p0;
            } else {
                node = node->children[1];
                pdf *= p1;
            }
        } else {
            return pdf;
        }
    }
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
    } else if (finite_lights.empty()) {
        split(&tree.root_, 0, uint32_t(infinite_lights.size()), infinite_lights, scene);
    } else {
        tree.root_.children[0] = new Tree::Node;
        tree.root_.children[1] = new Tree::Node;

        split(tree.root_.children[0], 0, uint32_t(finite_lights.size()), finite_lights, scene);
        split(tree.root_.children[1], 0, uint32_t(infinite_lights.size()), infinite_lights, scene);
    }

    tree.root_.gather();
}

void Tree_builder::split(Tree::Node* node, uint32_t begin, uint32_t end, Lights const& lights,
                         Scene const& scene) const noexcept {
    if (1 == end - begin) {
        uint32_t const l = lights[begin];

        auto const& light = scene.lights()[l];

        node->center = scene.light_center(l);
        node->power  = spectrum::luminance(light.power(AABB(float3(-1.f), float3(1.f)), scene));
        node->finite = light.is_finite(scene);
        node->light  = l;
    } else {
        node->children[0] = new Tree::Node;
        node->children[1] = new Tree::Node;

        if (scene.lights()[lights[begin]].is_finite(scene)) {
            AABB bb = AABB::empty();

            for (uint32_t i = begin; i < end; ++i) {
                bb.insert(scene.light_center(lights[i]));
            }

            uint32_t const axis = index_max_component(bb.extent());

            float const position = bb.position()[axis];

            Lights a;
            Lights b;

            for (uint32_t i = begin; i < end; ++i) {
                uint32_t const l = lights[i];

                float3 const c = scene.light_center(l);

                if (c[axis] < position) {
                    a.push_back(l);
                } else {
                    b.push_back(l);
                }
            }

            split(node->children[0], 0, uint32_t(a.size()), a, scene);
            split(node->children[1], 0, uint32_t(b.size()), b, scene);
        } else {
            uint32_t const middle = (end - begin) / 2;

            split(node->children[0], begin, middle, lights, scene);
            split(node->children[1], middle, end, lights, scene);
        }
    }
}

}  // namespace scene::light
