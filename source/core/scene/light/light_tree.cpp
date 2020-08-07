#include "light_tree.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/memory/align.hpp"
#include "base/spectrum/rgb.hpp"
#include "scene/scene.inl"
#include "scene/shape/shape.inl"

#include "base/debug/assert.hpp"

namespace scene::light {

void Build_node::gather(uint32_t const* orders, Build_node* nodes) {
    if (middle > 0) {
        uint32_t const children = children_or_light;

        Build_node& c0 = nodes[children];
        Build_node& c1 = nodes[children + 1];

        c0.gather(orders, nodes);
        c1.gather(orders, nodes);

        float const total_power = c0.power + c1.power;

        center = (c0.power * c0.center + c1.power * c1.center) / total_power;

        power = total_power;

        middle = c0.end;

        end = c1.end;
    } else {
        end = orders[children_or_light] + 1;
    }
}

Tree::Tree()
    : num_finite_lights_(0),
      num_infinite_lights_(0),
      nodes_(nullptr),
      light_orders_(nullptr),
      infinite_light_powers_(nullptr) {}

Tree::~Tree() {
    memory::free_aligned(infinite_light_powers_);
    memory::free_aligned(light_orders_);
    memory::free_aligned(nodes_);
}

float Tree::Node::weight(float3 const& p, float3 const& n, bool total_sphere) const {
    float3 const axis = center - p;

    float const base = power / std::max(squared_length(axis), 0.0001f);

    if (children | total_sphere) {
        return 0.5f * base;
    }

    float3 const na = normalize(axis);

    return std::max(dot(n, na), 0.01f) * base;
}

Tree::Result Tree::random_light(float3 const& p, float3 const& n, bool total_sphere,
                                float random) const {
    float const ip = infinite_weight_;

    if (random < infinite_guard_) {
        auto const l = infinite_light_distribution_.sample_discrete(random);

        return {l.offset, l.pdf * ip};
    }

    float pdf = 1.f - ip;

    random = (random - ip) / pdf;

    for (uint32_t nid = 0;;) {
        Node const& node = nodes_[nid];

        if (node.children) {
            uint32_t const c0 = node.children_or_light;
            uint32_t const c1 = c0 + 1;

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

            return {node.children_or_light, pdf};
        }
    }
}

float Tree::pdf(float3 const& p, float3 const& n, bool total_sphere, uint32_t id) const {
    float const ip = infinite_weight_;

    uint32_t const lo = light_orders_[id];

    if (lo < infinite_end_) {
        return ip * infinite_light_distribution_.pdf(lo);
    }

    float pdf = 1.f - ip;

    for (uint32_t nid = 0;;) {
        Node const& node = nodes_[nid];

        if (node.children) {
            uint32_t const c0 = node.children_or_light;
            uint32_t const c1 = c0 + 1;

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

void Tree::allocate(uint32_t num_finite_lights, uint32_t num_infinite_lights) {
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

Tree_builder::Tree_builder() : build_nodes_(nullptr) {}

Tree_builder::~Tree_builder() {
    delete[] build_nodes_;
}

void Tree_builder::build(Tree& tree, Scene const& scene) {
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
        delete[] build_nodes_;

        uint32_t const num_nodes = 2 * num_finite_lights - 1;

        build_nodes_ = new Build_node[num_nodes];

        current_node_ = 1;

        split(tree, 0, 0, num_finite_lights, finite_lights, scene);

        build_nodes_[0].gather(tree.light_orders_, build_nodes_);

        nodes_ = tree.nodes_;

        serialize(num_nodes);
    }

    float const p0 = infinite_total_power;
    float const p1 = finite_lights.empty() ? 0.f : build_nodes_[0].power;

    float const pt = p0 + p1;

    float const infinite_weight = p0 / pt;

    tree.infinite_weight_ = infinite_weight;

    // This is because I'm afraid of the 1.f == random case
    tree.infinite_guard_ = finite_lights.empty() ? 1.1f : infinite_weight;
}

void Tree_builder::split(Tree& tree, uint32_t node_id, uint32_t begin, uint32_t end, Lights& lights,
                         Scene const& scene) {
    Build_node& node = build_nodes_[node_id];

    uint32_t const len = end - begin;

    if (1 == len) {
        uint32_t const l = lights[begin];

        auto const& light = scene.light(l);

        node.center = scene.light_center(l);
        node.power  = spectrum::luminance(light.power(AABB(float3(-1.f), float3(1.f)), scene));
        node.middle = 0;
        node.children_or_light = l;

        tree.light_orders_[l] = light_order_++;
    } else if (2 == len) {
        uint32_t const child0 = current_node_;

        current_node_ += 2;

        node.middle            = 0xFFFFFFFF;
        node.children_or_light = child0;

        split(tree, child0, begin, begin + 1, lights, scene);
        split(tree, child0 + 1, end - 1, end, lights, scene);
    } else {
        uint32_t const child0 = current_node_;

        current_node_ += 2;

        node.middle            = 0xFFFFFFFF;
        node.children_or_light = child0;

        AABB bb = AABB::empty();

        for (uint32_t i = begin; i < end; ++i) {
            bb.insert(scene.light_center(lights[i]));
        }

        uint32_t const axis = index_max_component(bb.extent());

        std::sort(lights.begin() + begin, lights.begin() + end,
                  [&scene, axis](uint32_t a, uint32_t b) {
                      float3 const ac = scene.light_center(a);
                      float3 const bc = scene.light_center(b);

                      return ac[axis] < bc[axis];
                  });

        uint32_t const middle = begin + (len / 2);

        split(tree, child0, begin, middle, lights, scene);
        split(tree, child0 + 1, middle, end, lights, scene);
    }
}

void Tree_builder::serialize(uint32_t num_nodes) {
    for (uint32_t i = 0; i < num_nodes; ++i) {
        Build_node const& source = build_nodes_[i];

        Tree::Node& dest = nodes_[i];

        dest.center            = source.center;
        dest.power             = source.power;
        dest.children          = source.middle > 0 ? true : false;
        dest.middle            = source.middle;
        dest.children_or_light = source.children_or_light;
    }
}

}  // namespace scene::light
