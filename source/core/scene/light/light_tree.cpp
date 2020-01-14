#include "light_tree.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/memory/align.hpp"
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

        middle = children[0]->end;

        end = children[1]->end;
    } else {
        end = orders[light] + 1;
    }
}

Tree::Tree() noexcept
    : num_finite_lights_(0),
      num_infinite_lights_(0),
      nodes_(nullptr),
      light_orders_(nullptr),
      infinite_light_powers_(nullptr) {}

Tree::~Tree() noexcept {
    memory::free_aligned(infinite_light_powers_);
    memory::free_aligned(light_orders_);
    memory::free_aligned(nodes_);
}

float Tree::Node::weight(float3 const& p, float3 const& n, bool total_sphere) const noexcept {
    float3 const axis = center - p;

    float const base = power / std::max(squared_length(axis), 0.0001f);

    if (children | total_sphere) {
        return 0.5f * base;
    } else {
        float3 const na = normalize(axis);

        return std::max(dot(n, na), 0.01f) * base;
    }
}

Tree::Result Tree::random_light(float3 const& p, float3 const& n, bool total_sphere,
                                float random) const noexcept {
    float const ip = infinite_weight_;

    if (random < infinite_guard_) {
        auto const l = infinite_light_distribution_.sample_discrete(random);

        return {l.offset, l.pdf * ip};
    } else {
        float pdf = 1.f - ip;

        random = (random - ip) / pdf;

        for (uint32_t nid = 0;;) {
            Node const& node = nodes_[nid];

            if (node.children) {
                uint32_t const c0 = nid + 1;
                uint32_t const c1 = node.next_or_light;

                float p0 = nodes_[c0].weight(p, n, total_sphere);
                float p1 = nodes_[c1].weight(p, n, total_sphere);

                float const pt = p0 + p1;

                SOFT_ASSERT(pt > 0.f);

                p0 /= pt;
                p1 /= pt;

                if (random < p0) {
                    nid = c0;
                    pdf *= p0;
                    random /= p0;
                } else {
                    nid = c1;
                    pdf *= p1;
                    random = (random - p0) / p1;
                }
            } else {
                SOFT_ASSERT(std::isfinite(pdf) && pdf > 0.f);

                return {node.next_or_light, pdf};
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
        float pdf = 1.f - ip;

        for (uint32_t nid = 0;;) {
            Node const& node = nodes_[nid];

            if (node.children) {
                uint32_t const c0 = nid + 1;
                uint32_t const c1 = node.next_or_light;

                float const p0 = nodes_[c0].weight(p, n, total_sphere);
                float const p1 = nodes_[c1].weight(p, n, total_sphere);

                float const pt = p0 + p1;

                SOFT_ASSERT(pt > 0.f);

                if (lo < node.middle) {
                    nid = c0;
                    pdf *= p0 / pt;
                } else {
                    nid = c1;
                    pdf *= p1 / pt;
                }
            } else {
                SOFT_ASSERT(std::isfinite(pdf) && pdf > 0.f);

                return pdf;
            }
        }
    }
}

void Tree::allocate(uint32_t num_finite_lights, uint32_t num_infinite_lights) noexcept {
    uint32_t const num_lights = num_finite_lights + num_infinite_lights;

    uint32_t const current_num_lights = num_finite_lights_ + num_infinite_lights_;

    if (current_num_lights != num_lights) {
        memory::free_aligned(light_orders_);

        light_orders_ = memory::allocate_aligned<uint32_t>(num_lights);
    }

    if (num_finite_lights_ != num_finite_lights) {
        memory::free_aligned(nodes_);

        uint32_t const num_nodes = 2 * num_finite_lights - 1;

        nodes_ = memory::allocate_aligned<Node>(num_nodes);
    }

    if (num_infinite_lights_ != num_infinite_lights) {
        memory::free_aligned(infinite_light_powers_);

        infinite_light_powers_ = memory::allocate_aligned<float>(num_infinite_lights);
    }

    num_finite_lights_   = num_finite_lights;
    num_infinite_lights_ = num_infinite_lights;
}

void Tree_builder::build(Tree& tree, Scene const& scene) noexcept {
    Lights finite_lights;
    Lights infinite_lights;

    light_order_ = 0;

    for (uint32_t l = 0, len = scene.num_lights(); l < len; ++l) {
        auto const& light = scene.light(l);

        if (light.is_finite(scene)) {
            finite_lights.push_back(l);
        } else {
            infinite_lights.push_back(l);
        }
    }

    Build_node root;

    uint32_t const num_finite_lights = uint32_t(finite_lights.size());

    uint32_t const num_infinite_lights = uint32_t(infinite_lights.size());

    tree.allocate(num_finite_lights, num_infinite_lights);

    float infinite_total_power = 0.f;

    for (uint32_t i = 0; i < num_infinite_lights; ++i) {
        uint32_t const l = infinite_lights[i];

        auto const& light = scene.light(l);

        float const power = spectrum::luminance(light.power(scene.aabb(), scene));

        tree.infinite_light_powers_[i] = power;

        tree.light_orders_[l] = light_order_++;

        infinite_total_power += power;
    }

    tree.infinite_end_ = light_order_;

    tree.infinite_light_distribution_.init(tree.infinite_light_powers_, num_infinite_lights);

    if (!finite_lights.empty()) {
        split(tree, &root, 0, num_finite_lights, finite_lights, scene);

        root.gather(tree.light_orders_);

        nodes_   = tree.nodes_;
        current_ = 0;

        serialize(&root);
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

        auto const& light = scene.light(l);

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

        std::sort(lights.begin() + begin, lights.begin() + end,
                  [&scene, axis](uint32_t a, uint32_t b) noexcept {
                      float3 const ac = scene.light_center(a);
                      float3 const bc = scene.light_center(b);

                      return ac[axis] < bc[axis];
                  });

        uint32_t const middle = begin + (len / 2);

        split(tree, node->children[0], begin, middle, lights, scene);
        split(tree, node->children[1], middle, end, lights, scene);
    }
}

void Tree_builder::serialize(Build_node* node) noexcept {
    auto& n = nodes_[current_++];

    if (node->children[0]) {
        serialize(node->children[0]);

        n.center        = node->center;
        n.power         = node->power;
        n.children      = true;
        n.middle        = node->middle;
        n.next_or_light = current_;

        serialize(node->children[1]);
    } else {
        n.center        = node->center;
        n.power         = node->power;
        n.children      = false;
        n.middle        = 0;
        n.next_or_light = node->light;
    }
}

}  // namespace scene::light
