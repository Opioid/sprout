#include "light_tree.hpp"
#include "base/math/distribution/distribution_1d.inl"
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

        center = (c0.center + c1.center) / 2.f;
    //    center = (c0.power * c0.center + c1.power * c1.center) / total_power;
    //    center = (c0.power * c0.power * c0.center + c1.power * c1.power * c1.center) / (c0.power * c0.power + c1.power * c1.power);

        power = total_power;

        middle = c0.end;

        end = c1.end;

        box = c0.box.merge(c1.box);
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
    delete[] infinite_light_powers_;
    delete[] light_orders_;
    delete[] nodes_;
}

float Tree::Node::weight(float3 const& p, float3 const& n, bool total_sphere) const {
    float3 const axis = center - p;

    bool const inside = length(axis) < radius;

    float sql = squared_length(axis);

    float const l = std::sqrt(sql);

    if (middle > 0) {
        sql = std::max(0.5f * radius, sql);
    }

    float const base = power / std::max(/*squared_length(axis)*/sql, 0.01f);

    if (total_sphere) {
        return 0.5f * base;
    }


//    if (((middle > 0) & inside)     | total_sphere) {
//        return 0.5f * base;
//    }

    float const cu = radius > 0.f ? std::asin(std::min(l / radius, 1.f)) : 0.f;

    float d = 1.f;

        float3 const na = normalize(axis);

    if (0 == middle) {
        float3 const da = cone.xyz();

        float const cos = -dot(da, na);

        float const a = std::acos(cos);

        float const cs = std::max(a - cone[3] - cu, 0.f);

        if (cs < Pi / 2) {
            d = std::cos(cs);
        } else {
            d = 0.f;
        }

//        if (a > cone[3]) {
//            d -= 1.f - std::max(std::cos(a - cone[3]), 0.f);
//        }
    }



    float const angle = std::max(std::cos(std::acos(dot(n, na)) - cu), 0.01f);

    return d * angle * base;
}

Tree::Result Tree::random_light(float3 const& p, float3 const& n, bool total_sphere,
                                float random) const {
    float const ip = infinite_weight_;

    if (random < infinite_guard_) {
        auto const l = infinite_light_distribution_.sample_discrete(random);

        float const pdf = l.pdf * ip;

        SOFT_ASSERT(pdf > 0.f);

        return {l.offset, pdf};
    }

    float pdf = 1.f - ip;

    random = (random - ip) / pdf;

    for (uint32_t nid = 0;;) {
        Node const& node = nodes_[nid];

        if (node.middle > 0) {
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

        if (node.middle > 0) {
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
        delete[] light_orders_;

        light_orders_ = new uint32_t[num_lights];
    }

    if (num_finite_lights_ != num_finite_lights) {
        delete[] nodes_;

        uint32_t const num_nodes = 2 * num_finite_lights - 1;

        nodes_ = new Node[num_nodes];
    }

    if (num_infinite_lights_ != num_infinite_lights) {
        delete[] infinite_light_powers_;

        infinite_light_powers_ = new float[num_infinite_lights];
    }

    num_finite_lights_   = num_finite_lights;
    num_infinite_lights_ = num_infinite_lights;
}

Tree_builder::Tree_builder() : build_nodes_(nullptr), candidates_(nullptr) {}

Tree_builder::~Tree_builder() {
    delete [] candidates_;
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

        float const power = /*spectrum::luminance*/average(light.power(scene.aabb(), scene));

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

        delete [] candidates_;

        if (num_finite_lights > 2) {
        candidates_ = new Split_candidate[num_finite_lights - 2];
        } else {
            candidates_ = nullptr;
        }

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
        node.cone   = scene.light_cone(l);
        node.power  = /*spectrum::luminance*/average(light.power(AABB(float3(-1.f), float3(1.f)), scene));
        node.middle = 0;
        node.children_or_light = l;
        node.box = AABB(node.center, node.center);

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

        float total_power = 0.f;

        for (uint32_t i = begin; i < end; ++i) {
            uint32_t const l = lights[i];

            total_power +=  average(scene.light(l).power(AABB(float3(-1.f), float3(1.f)), scene));

            bb.insert(scene.light_center(l));
        }

        float const aabb_surface_area = bb.surface_area();

        uint32_t const axis = index_max_component(bb.extent());

        std::sort(lights.begin() + begin, lights.begin() + end,
                  [&scene, axis](uint32_t a, uint32_t b) noexcept {
                      float3 const ac = scene.light_center(a);
                      float3 const bc = scene.light_center(b);

                      return ac[axis] < bc[axis];
                  });


        for (uint32_t i = begin + 1, j = 0; i < end; ++i, ++j) {
            candidates_[j].init(begin, end, i, total_power, aabb_surface_area, lights, scene);
        }

        std::sort(candidates_, candidates_ + len - 1, [](Split_candidate const& a, Split_candidate const& b) noexcept {
           return a.weight < b.weight;
        });


        /*
        float const threshold = bb.position()[axis];

        uint32_t s = begin + 1;
        for (; s < end - 1; ++s) {
            if (scene.light_center(lights[s])[axis] > threshold) {
                break;
            }
        }
        */

        uint32_t const middle = candidates_[0].split_node;//*/ begin + (len / 2);

        split(tree, child0, begin, middle, lights, scene);
        split(tree, child0 + 1, middle, end, lights, scene);
    }
}

Tree_builder::Split_candidate::Split_candidate() = default;

void Tree_builder::Split_candidate::init(uint32_t begin, uint32_t end, uint32_t split, float total_power, float aabb_surface_area, Lights const& lights, Scene const& scene) {

    float power_a = 0.f;
    float power_b = 0.f;

    for (uint32_t i = begin; i < split; ++i) {
        uint32_t const l = lights[i];
        power_a += average(scene.light(l).power(AABB(float3(-1.f), float3(1.f)), scene));
    }

    for (uint32_t i = split; i < end; ++i) {
        uint32_t const l = lights[i];
        power_b += average(scene.light(l).power(AABB(float3(-1.f), float3(1.f)), scene));
    }

    split_node = split;
 //   weight = std::abs(power_a - power_b);



    AABB a = AABB::empty();
    AABB b = AABB::empty();

    for (uint32_t i = begin; i < split; ++i) {
        uint32_t const l = lights[i];
        a.insert(scene.light_center(l));
    }

    for (uint32_t i = split; i < end; ++i) {
        uint32_t const l = lights[i];
        b.insert(scene.light_center(l));
    }

    split_node = split;

    weight = 0.5f * std::abs(power_a - power_b) + (a.surface_area() + b.surface_area());

  //  weight = (/*power_a **/ a.surface_area() + /*power_b **/ b.surface_area()) / (aabb_surface_area);
}

void Tree_builder::serialize(uint32_t num_nodes) {
    for (uint32_t i = 0; i < num_nodes; ++i) {
        Build_node const& source = build_nodes_[i];

        Tree::Node& dest = nodes_[i];

        dest.center            = /*source.center;//*/source.box.position();
        dest.cone              = source.cone;
        dest.radius            = 0.5f * std::max(length(source.center - source.box.min()), length(source.center - source.box.max()));
        dest.power             = source.power;
        dest.middle            = source.middle;
        dest.children_or_light = source.children_or_light;
    }
}

}  // namespace scene::light
