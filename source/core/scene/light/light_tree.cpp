#include "light_tree.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/memory/align.hpp"
#include "base/memory/array.inl"
#include "base/spectrum/rgb.hpp"
#include "scene/scene.inl"

#include "base/debug/assert.hpp"

namespace scene::light {

Build_node::Build_node() noexcept : children{nullptr, nullptr} {}

Build_node::~Build_node() {
    delete children[0];
    delete children[1];
}

void Build_node::gather(uint32_t const* orders) noexcept {
    if (children[0]) {
        children[0]->gather(orders);
        children[1]->gather(orders);

        float const total_power = children[0]->power + children[1]->power;

        center = (children[0]->power * children[0]->center +
                  children[1]->power * children[1]->center) /
                 total_power;

        power = total_power;

        end = children[1]->end;
    } else {
        end = orders[light] + 1;
    }
}

float Build_node::weight(float3 const& p) const noexcept {
    return power / squared_distance(center, p);
}

Tree::Tree() noexcept {}

Tree::~Tree() noexcept {}

Tree::Result Tree::random_light(float3 const& p, float random) const noexcept {
    float const ip = infinite_weight_;

    if (random < ip) {
        auto const l = infinite_light_distribution_.sample_discrete(random);
        return {l.offset, l.pdf * ip};
    } else {
        float pdf = 1.f - ip;

        Build_node const* node = &root_;

        for (;;) {
            if (node->children[0]) {
                float p0 = node->children[0]->weight(p);
                float p1 = node->children[1]->weight(p);

                float const pt = p0 + p1;

                SOFT_ASSERT(pt > 0.f);

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
                SOFT_ASSERT(std::isfinite(pdf));

                return {node->light, pdf};
            }
        }
    }
}

float Tree::pdf(float3 const& p, uint32_t id) const noexcept {
    float const ip = infinite_weight_;

    uint32_t const lo = light_orders_[id];

    if (lo < infinite_end_) {
        return ip * infinite_light_distribution_.pdf(lo);
    } else {
        Build_node const* node = &root_;

        float pdf = 1.f - ip;

        for (;;) {
            if (node->children[0]) {
                float const p0 = node->children[0]->weight(p);
                float const p1 = node->children[1]->weight(p);

                float const pt = p0 + p1;

                SOFT_ASSERT(pt > 0.f);

                if (lo < node->children[0]->end) {
                    node = node->children[0];
                    pdf *= p0 / pt;
                } else {
                    node = node->children[1];
                    pdf *= p1 / pt;
                }
            } else {
                SOFT_ASSERT(std::isfinite(pdf));

                return pdf;
            }
        }
    }
}

void Tree_builder::build(Tree& tree, Scene const& scene) noexcept {
    delete tree.root_.children[0];
    delete tree.root_.children[1];

    tree.root_.children[0] = nullptr;
    tree.root_.children[1] = nullptr;

    Lights finite_lights;
    Lights infinite_lights;

    uint32_t const num_lights = uint32_t(scene.lights().size());

    tree.light_orders_.resize(num_lights);

    light_order_ = 0;

    for (uint32_t l = 0; l < num_lights; ++l) {
        auto const& light = scene.lights()[l];

        if (light.is_finite(scene)) {
            finite_lights.push_back(l);
        } else {
            infinite_lights.push_back(l);
        }
    }

    Build_node& root = tree.root_;

    uint32_t const num_infinite_lights = uint32_t(infinite_lights.size());

    tree.infinite_light_powers_.resize(num_infinite_lights);

    float infinite_total_power = 0.f;

    for (uint32_t i = 0; i < num_infinite_lights; ++i) {
        uint32_t const l = infinite_lights[i];

        auto const& light = scene.lights()[l];

        float const power = std::sqrt(spectrum::luminance(light.power(scene.aabb(), scene)));

        tree.infinite_light_powers_[i] = power;

        tree.light_orders_[l] = light_order_++;

        infinite_total_power += power;
    }

    tree.infinite_end_ = light_order_;

    tree.infinite_light_distribution_.init(tree.infinite_light_powers_.data(), num_infinite_lights);

    split(tree, &root, 0, uint32_t(finite_lights.size()), finite_lights, scene);

    root.gather(tree.light_orders_.data());

    float const p0 = infinite_total_power;
    float const p1 = root.power;

    float const pt = p0 + p1;

    tree.infinite_weight_ = p0 / pt;
}

void Tree_builder::split(Tree& tree, Build_node* node, uint32_t begin, uint32_t end,
                         Lights const& lights, Scene const& scene) noexcept {
    if (1 == end - begin) {
        uint32_t const l = lights[begin];

        auto const& light = scene.lights()[l];

        node->center = scene.light_center(l);
        node->power  = spectrum::luminance(light.power(AABB(float3(-1.f), float3(1.f)), scene));
        node->light  = l;

        tree.light_orders_[l] = light_order_++;
    } else {
        node->children[0] = new Build_node;
        node->children[1] = new Build_node;

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

            split(tree, node->children[0], 0, uint32_t(a.size()), a, scene);
            split(tree, node->children[1], 0, uint32_t(b.size()), b, scene);
        } else {
            uint32_t const middle = (end - begin) / 2;

            split(tree, node->children[0], begin, middle, lights, scene);
            split(tree, node->children[1], middle, end, lights, scene);
        }
    }
}

}  // namespace scene::light
