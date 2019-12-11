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

float Build_node::weight(float3 const& p, float3 const& n, bool total_sphere) const noexcept {
    const float base = power / std::max(squared_distance(center, p), 0.0001f);

    if ((nullptr != children[0]) | total_sphere) {
        return 0.5f * base;
    } else {
        float3 const na = normalize(center - p);

        return std::max(dot(n, na), 0.01f) * base;
    }
}

Tree::Tree() noexcept {}

Tree::~Tree() noexcept {}

Tree::Result Tree::random_light(float3 const& p, float3 const& n, bool total_sphere,
                                float random) const noexcept {
    float const ip = infinite_weight_;

    if (random < infinite_guard_) {
        auto const l = infinite_light_distribution_.sample_discrete(random);

        return {l.offset, l.pdf * ip};
    } else {
        float pdf = 1.f - ip;

        Build_node const* node = &root_;

        for (uint32_t i = 0;; ++i) {
            if (node->children[0]) {
                float p0 = node->children[0]->weight(p, n, total_sphere);
                float p1 = node->children[1]->weight(p, n, total_sphere);

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
                SOFT_ASSERT(std::isfinite(pdf) && pdf > 0.f);

                return {node->light, pdf};
            }
        }
    }
}

float Tree::pdf(float3 const& p, float3 const& n, bool total_sphere, uint32_t id) const noexcept {
    float const ip = infinite_weight_;

    uint32_t const lo = light_orders_[id];

    if (lo < infinite_end_) {
        return ip * infinite_light_distribution_.pdf(lo);
    } else {
        Build_node const* node = &root_;

        float pdf = 1.f - ip;

        for (;;) {
            if (node->children[0]) {
                float const p0 = node->children[0]->weight(p, n, total_sphere);
                float const p1 = node->children[1]->weight(p, n, total_sphere);

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
                SOFT_ASSERT(std::isfinite(pdf) && pdf > 0.f);

                return pdf;
            }
        }
    }
}

void Tree_builder::build(Tree& tree, Scene const& scene) noexcept {
    delete tree.root_.children[0];
    delete tree.root_.children[1];

    tree.root_.power = 0.f;

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

    if (!finite_lights.empty()) {
        split(tree, &root, 0, uint32_t(finite_lights.size()), finite_lights, scene);

        root.gather(tree.light_orders_.data());
    }

    float const p0 = infinite_total_power;
    float const p1 = root.power;

    float const pt = p0 + p1;

    float const infinite_weight = p0 / pt;

    tree.infinite_weight_ = infinite_weight;

    // This is because I'm afraid of the 1.f == random case
    tree.infinite_guard_ = finite_lights.empty() ? 1.1f : infinite_weight;
}

void Tree_builder::split(Tree& tree, Build_node* node, uint32_t begin, uint32_t end, Lights& lights,
                         Scene const& scene) noexcept {
    uint32_t const len = end - begin;

    if (1 == len) {
        uint32_t const l = lights[begin];

        auto const& light = scene.lights()[l];

        node->center = scene.light_center(l);
        node->power  = spectrum::luminance(light.power(AABB(float3(-1.f), float3(1.f)), scene));
        node->light  = l;

        tree.light_orders_[l] = light_order_++;
    } else if (2 == len) {
        node->children[0] = new Build_node;
        node->children[1] = new Build_node;

        split(tree, node->children[0], begin, begin + 1, lights, scene);
        split(tree, node->children[1], end - 1, end, lights, scene);
    } else {
        node->children[0] = new Build_node;
        node->children[1] = new Build_node;

        AABB bb = AABB::empty();

        for (uint32_t i = begin; i < end; ++i) {
            bb.insert(scene.light_center(lights[i]));
        }

        uint32_t const axis = index_max_component(bb.extent());

        std::sort(
            lights.begin() + begin, lights.begin() + end,
            [&scene, axis ](uint32_t a, uint32_t b) noexcept {
                float3 const ac = scene.light_center(a);
                float3 const bc = scene.light_center(b);

                return ac[axis] < bc[axis];
            });

        uint32_t const middle = begin + (len / 2);

        split(tree, node->children[0], begin, middle, lights, scene);
        split(tree, node->children[1], middle, end, lights, scene);
    }
}

}  // namespace scene::light
