#include "light_tree.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/memory/array.inl"
#include "base/spectrum/rgb.hpp"
#include "scene/material/material_sample_helper.hpp"
#include "scene/scene.inl"
#include "scene/shape/shape.inl"

#include "base/debug/assert.hpp"

// More or less complete implementation of
// Importance Sampling of Many Lights with Adaptive Tree Splitting
// http://aconty.com/pdf/many-lights-hpg2018.pdf

namespace scene::light {

void Build_node::count_max_splits(uint32_t depth, Build_node* nodes, uint32_t& splits) {
    if (0 == middle) {
        if (depth < Tree::Max_split_depth) {
            splits += num_lights;
        } else {
            ++splits;
        }
    } else {
        if (depth == Tree::Max_split_depth - 1) {
            splits += 2;
        } else {
            nodes[children_or_light].count_max_splits(depth + 1, nodes, splits);
            nodes[children_or_light + 1].count_max_splits(depth + 1, nodes, splits);
        }
    }
}

class Traversal_stack {
  public:
    Traversal_stack() = default;

    struct Node {
        float pdf;
        float random;

        uint32_t node;
        uint32_t depth;
    };

    bool empty() const {
        return 0 == end_;
    }

    void push(Node const& value) {
        SOFT_ASSERT(end_ < Stack_size);

        stack_[end_++] = value;
    }

    Node pop() {
        return stack_[--end_];
    }

  private:
    static uint32_t constexpr Stack_size = (1 << (Tree::Max_split_depth - 1)) + 1;

    uint32_t end_ = 0;

    Node stack_[Stack_size];
};

static float4 cone_union(float4 a, float4 b) {
    if (float4(1.f) == a) {
        return b;
    }

    float a_angle = std::acos(a[3]);
    float b_angle = std::acos(b[3]);

    if (b_angle > a_angle) {
        std::swap(a, b);
        std::swap(a_angle, b_angle);
    }

    float const d_angle = std::acos(dot(a.xyz(), b.xyz()));

    if (std::min(d_angle + b_angle, Pi) <= a_angle) {
        return a;
    }

    float const o_angle = (a_angle + d_angle + b_angle) / 2.f;

    if (Pi <= o_angle) {
        float4(a.xyz(), -1.f);
    }

    float const r_angle = o_angle - a_angle;

    float3x3 rot;
    set_rotation(rot, cross(a.xyz(), b.xyz()), r_angle);

    float3 const axis = normalize(transform_vector(rot, a.xyz()));

    return float4(axis, std::cos(o_angle));
}

// 0.08 ^ 4
float Tree::splitting_threshold_ = 0.00004096f;

void Tree::set_splitting_threshold(float st) {
    splitting_threshold_ = pow4(st);
}

Tree::Tree()
    : num_lights_(0),
      num_infinite_lights_(0),
      num_nodes_(0),
      nodes_(nullptr),
      light_orders_(nullptr),
      light_mapping_(nullptr),
      infinite_light_powers_(nullptr) {}

Tree::~Tree() {
    delete[] infinite_light_powers_;
    delete[] light_mapping_;
    delete[] light_orders_;
    delete[] nodes_;
}

static inline float clamped_cos_sub(float cos_a, float cos_b, float sin_a, float sin_b) {
    float const angle = cos_a * cos_b + sin_a * sin_b;
    return (cos_a > cos_b) ? 1.f : angle;
}

static inline float clamped_sin_sub(float cos_a, float cos_b, float sin_a, float sin_b) {
    float const angle = sin_a * cos_b - sin_b * cos_a;
    return (cos_a > cos_b) ? 0.f : angle;
}

static inline float importance(float3_p center, float3_p p, float3_p n, float4_p cone, float radius,
                               float power, bool total_sphere) {
    float3 const axis = center - p;

    float const l = std::max(length(axis), 0.0001f);

    float const sin_cu = std::min(radius / l, 1.f);
    float const cos_cu = std::sqrt(1.f - sin_cu * sin_cu);

    float3 const na = axis / l;
    float3 const da = cone.xyz();

    float const cos_cone = cone[3];
    float const sin_cone = std::sqrt(1.f - cos_cone * cos_cone);

    float const cos_a = -dot(da, na);
    float const sin_a = std::sqrt(std::max(1.f - cos_a * cos_a, 0.f));

    float const d0 = clamped_cos_sub(cos_a, cos_cone, sin_a, sin_cone);
    float const d1 = clamped_sin_sub(cos_a, cos_cone, sin_a, sin_cone);
    float const d2 = std::max(clamped_cos_sub(d0, cos_cu, d1, sin_cu), 0.f);

    float const d_min = std::max(0.5f * radius, l);
    float const base  = power / (d_min * d_min);

    if (total_sphere) {
        return std::max(d2 * base, 0.001f);
    }

    float const cos_n = saturate(dot(n, na));
    float const sin_n = std::sqrt(1.f - cos_n * cos_n);
    float const angle = clamped_cos_sub(cos_n, cos_cu, sin_n, sin_cu);

    return std::max(d2 * angle * base, 0.001f);
}

static inline float importance(float3_p center, float3_p p0, float3_p p1, float3_p dir,
                               float4_p cone, float radius, float power) {
    float3 const axis = p0 - center;

    float3 const v0 = normalize(axis);
    float3 const v1 = normalize(p1 - center);

    float3 const o0  = v0;
    float3 const uv2 = cross(v0, v1);

    float const uv2l = length(uv2);

    if (uv2l < 0.00001f) {
        return power;
    }

    float3 const v2 = uv2 / uv2l;
    float3 const o1 = cross(o0, v2);

    float3 const da = cone.xyz();

    float const cos_a0 = scene::material::clamp_dot(o0, da);
    float const cos_a1 = scene::material::clamp_dot(o1, da);

    float const cos_phi = cos_a0 / std::sqrt(cos_a0 * cos_a0 + cos_a1 * cos_a1);
    float const sin_phi = std::sqrt(std::max(1.f - cos_phi * cos_phi, 0.f));

    float const delta = -dot(axis, dir);

    float3 const closest_point = p0 + delta * dir;

    float const l = std::max(distance(closest_point, center), 0.0001f);

    float const sin_cu = std::min(radius / l, 1.f);
    float const cos_cu = std::sqrt(1.f - sin_cu * sin_cu);

    float cos_theta_min;

    if (cos_a1 < 0.f || dot(v0, v1) > cos_phi) {
        float const b_max = std::max(dot(v0, da), dot(v1, da));

        cos_theta_min = std::min(b_max, 1.f);
    } else {
        float3 const v = cos_phi * o0 + sin_phi * o1;

        cos_theta_min = clamp(dot(v, da), -1.f, 1.f);
    }

    float const cos_cone = cone[3];
    float const sin_cone = std::sqrt(1.f - cos_cone * cos_cone);

    float const cos_a = cos_theta_min;
    float const sin_a = std::sqrt(1.f - cos_a * cos_a);

    float const d0 = clamped_cos_sub(cos_a, cos_cone, sin_a, sin_cone);
    float const d1 = clamped_sin_sub(cos_a, cos_cone, sin_a, sin_cone);
    float const d2 = std::max(clamped_cos_sub(d0, cos_cu, d1, sin_cu), 0.f);

    return std::max((d2 * power) / l, 0.001f);
}

static float light_weight(float3_p p, float3_p n, bool total_sphere, uint32_t light,
                          Scene const& scene) {
    float3 const center = scene.light_aabb(light).position();
    float4 const cone   = scene.light_cone(light);

    float const radius = 0.5f * length(scene.light_aabb(light).extent());
    float const power  = scene.light_power(light);

    return importance(center, p, n, cone, radius, power, total_sphere);
}

static float light_weight(float3_p p0, float3_p p1, float3_p dir, uint32_t light,
                          Scene const& scene) {
    float3 const center = scene.light_aabb(light).position();
    float4 const cone   = scene.light_cone(light);

    float const radius = 0.5f * length(scene.light_aabb(light).extent());
    float const power  = scene.light_power(light);

    return importance(center, p0, p1, dir, cone, radius, power);
}

float Tree::Node::weight(float3_p p, float3_p n, bool total_sphere) const {
    float const r = center[3];

    return importance(center.xyz(), p, n, cone, r, power, total_sphere);
}

float Tree::Node::weight(float3_p p0, float3_p p1, float3_p dir) const {
    float const r = center[3];

    return importance(center.xyz(), p0, p1, dir, cone, r, power);
}

bool Tree::Node::split(float3_p p) const {
    float const r = center[3];
    float const d = distance(p, center.xyz());

    float const a = std::max(d - r, 0.001f);
    float const b = d + r;

    float const eg  = 1.f / (a * b);
    float const eg2 = eg * eg;

    float const a3 = a * a * a;
    float const b3 = b * b * b;

    float const e2g = (b3 - a3) / (3.f * (b - a) * a3 * b3);

    float const vg = e2g - eg2;

    float const ve = variance;
    float const ee = power / float(num_lights);

    float const s2 = (ve * vg + ve * eg2 + ee * ee * vg);
    float const ns = 1.f / (1.f + std::sqrt(s2));
    // float const ns = std::pow(1.f / (1.f + std::sqrt(s2)), 1.f / 4.f);

    return ns < Tree::splitting_threshold_;
}

bool Tree::Node::split(float3_p p0, float3_p dir) const {
    float3 const axis = p0 - center.xyz();

    float const delta = -dot(axis, dir);

    float3 const closest_point = p0 + delta * dir;

    return split(closest_point);
}

Light_pick Tree::Node::random_light(float3_p p, float3_p n, bool total_sphere, float random,
                                    uint32_t const* const light_mapping, Scene const& scene) const {
    if (1 == num_lights) {
        return {light_mapping[children_or_light], 1.f};
    }

    float weights[4] = {0.f, 0.f, 0.f, 0.f};

    for (uint32_t i = 0, len = num_lights; i < len; ++i) {
        weights[i] = light_weight(p, n, total_sphere, light_mapping[children_or_light + i], scene);
    }

    auto const l = distribution_sample_discrete<4>(weights, num_lights, random);

    return {light_mapping[children_or_light + l.offset], l.pdf};
}

Light_pick Tree::Node::random_light(float3_p p0, float3_p p1, float3_p dir, float random,
                                    uint32_t const* const light_mapping, Scene const& scene) const {
    if (1 == num_lights) {
        return {light_mapping[children_or_light], 1.f};
    }

    float weights[4] = {0.f, 0.f, 0.f, 0.f};

    for (uint32_t i = 0, len = num_lights; i < len; ++i) {
        weights[i] = light_weight(p0, p1, dir, light_mapping[children_or_light + i], scene);
    }

    auto const l = distribution_sample_discrete<4>(weights, num_lights, random);

    return {light_mapping[children_or_light + l.offset], l.pdf};
}

float Tree::Node::pdf(float3_p p, float3_p n, bool total_sphere, uint32_t id,
                      uint32_t const* const light_mapping, Scene const& scene) const {
    if (1 == num_lights) {
        return 1.f;
    }

    float weights[4] = {0.f, 0.f, 0.f, 0.f};

    for (uint32_t i = 0, len = num_lights; i < len; ++i) {
        weights[i] = light_weight(p, n, total_sphere, light_mapping[children_or_light + i], scene);
    }

    return distribution_pdf<4>(weights, id - children_or_light);
}

void Tree::random_light(float3_p p, float3_p n, bool total_sphere, float random, bool split,
                        Scene const& scene, Lights& lights) const {
    lights.clear();

    float ip = 0.f;

    uint32_t depth_bias = 0;

    uint32_t const num_infinite_lights = num_infinite_lights_;

    if (split && num_infinite_lights < Max_lights - 1) {
        depth_bias = infinite_depth_bias_;

        for (uint32_t i = 0; i < num_infinite_lights; ++i) {
            lights.push_back({light_mapping_[i], 1.f});
        }

        if (0 == num_nodes_) {
            return;
        }
    } else {
        ip = infinite_weight_;

        if (random < infinite_guard_) {
            auto const l = infinite_light_distribution_.sample_discrete(random);

            float const pdf = l.pdf * ip;

            SOFT_ASSERT(pdf > 0.f);

            lights.push_back({light_mapping_[l.offset], pdf});

            return;
        }
    }

    float const pdf = 1.f - ip;

    Traversal_stack stack;

    Traversal_stack::Node t{pdf, (random - ip) / pdf, 0, depth_bias};

    stack.push(t);

    while (!stack.empty()) {
        Node const& node = nodes_[t.node];

        bool const do_split = split && t.depth < Max_split_depth && node.split(p);

        if (node.middle > 0) {
            uint32_t const c0 = node.children_or_light;
            uint32_t const c1 = c0 + 1;

            if (do_split) {
                t.node = c0;
                stack.push({t.pdf, t.random, c1, ++t.depth});
            } else {
                float p0 = nodes_[c0].weight(p, n, total_sphere);
                float p1 = nodes_[c1].weight(p, n, total_sphere);

                float const pt = p0 + p1;

                SOFT_ASSERT(pt > 0.f);

                p0 /= pt;
                p1 /= pt;

                if (t.random < p0) {
                    t.node = c0;
                    t.pdf *= p0;
                    t.random /= p0;
                } else {
                    t.node = c1;
                    t.pdf *= p1;
                    t.random = (t.random - p0) / p1;
                }

                split = false;
            }
        } else {
            if (do_split) {
                for (uint32_t i = 0, len = node.num_lights; i < len; ++i) {
                    lights.push_back({light_mapping_[node.children_or_light + i], t.pdf});
                }
            } else {
                SOFT_ASSERT(std::isfinite(t.pdf));
                SOFT_ASSERT(t.pdf > 0.f);

                Light_pick const pick = node.random_light(p, n, total_sphere, t.random,
                                                          light_mapping_, scene);

                lights.push_back({pick.id, pick.pdf * t.pdf});
            }

            t = stack.pop();

            split = true;
        }
    }
}

void Tree::random_light(float3_p p0, float3_p p1, float random, bool split, Scene const& scene,
                        Lights& lights) const {
    lights.clear();

    float ip = 0.f;

    uint32_t depth_bias = 0;

    uint32_t const num_infinite_lights = num_infinite_lights_;

    if (split && num_infinite_lights < Max_lights - 1) {
        depth_bias = infinite_depth_bias_;

        for (uint32_t i = 0; i < num_infinite_lights; ++i) {
            lights.push_back({light_mapping_[i], 1.f});
        }

        if (0 == num_nodes_) {
            return;
        }
    } else {
        ip = infinite_weight_;

        if (random < infinite_guard_) {
            auto const l = infinite_light_distribution_.sample_discrete(random);

            float const pdf = l.pdf * ip;

            SOFT_ASSERT(pdf > 0.f);

            lights.push_back({light_mapping_[l.offset], pdf});

            return;
        }
    }

    float3 const dir = normalize(p0 - p1);

    float const pdf = 1.f - ip;

    Traversal_stack stack;

    Traversal_stack::Node t{pdf, (random - ip) / pdf, 0, depth_bias};

    stack.push(t);

    while (!stack.empty()) {
        Node const& node = nodes_[t.node];

        bool const do_split = split && t.depth < Max_split_depth && node.split(p0, dir);

        if (node.middle > 0) {
            uint32_t const c0 = node.children_or_light;
            uint32_t const c1 = c0 + 1;

            if (do_split) {
                t.node = c0;
                stack.push({t.pdf, t.random, c1, ++t.depth});
            } else {
                float pr0 = nodes_[c0].weight(p0, p1, dir);
                float pr1 = nodes_[c1].weight(p0, p1, dir);

                float const prt = pr0 + pr1;

                SOFT_ASSERT(prt > 0.f);

                pr0 /= prt;
                pr1 /= prt;

                if (t.random < pr0) {
                    t.node = c0;
                    t.pdf *= pr0;
                    t.random /= pr0;
                } else {
                    t.node = c1;
                    t.pdf *= pr1;
                    t.random = (t.random - pr0) / pr1;
                }

                split = false;
            }
        } else {
            if (do_split) {
                for (uint32_t i = 0, len = node.num_lights; i < len; ++i) {
                    lights.push_back({light_mapping_[node.children_or_light + i], t.pdf});
                }
            } else {
                SOFT_ASSERT(std::isfinite(t.pdf));
                SOFT_ASSERT(t.pdf > 0.f);

                Light_pick const pick = node.random_light(p0, p1, dir, t.random, light_mapping_,
                                                          scene);

                lights.push_back({pick.id, pick.pdf * t.pdf});
            }

            t = stack.pop();

            split = true;
        }
    }
}

float Tree::pdf(float3_p p, float3_p n, bool total_sphere, bool split, uint32_t id,
                Scene const& scene) const {
    uint32_t const lo = light_orders_[id];

    uint32_t const num_infinite_lights = num_infinite_lights_;

    bool const split_infinite = split && num_infinite_lights < Max_lights - 1;

    if (lo < infinite_end_) {
        if (split_infinite) {
            return 1.f;
        } else {
            return infinite_weight_ * infinite_light_distribution_.pdf(lo);
        }
    }

    float const ip = split_infinite ? 0.f : infinite_weight_;

    float pdf = 1.f - ip;

    SOFT_ASSERT(pdf > 0.f);

    for (uint32_t nid = 0, depth = infinite_depth_bias_;; ++depth) {
        Node const& node = nodes_[nid];

        bool const do_split = split && depth < Max_split_depth && node.split(p);

        if (node.middle > 0) {
            uint32_t const c0 = node.children_or_light;
            uint32_t const c1 = c0 + 1;

            if (do_split) {
                if (lo < node.middle) {
                    nid = c0;
                } else {
                    nid = c1;
                }
            } else {
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

                split = false;
            }
        } else {
            if (do_split) {
                return pdf;
            } else {
                SOFT_ASSERT(std::isfinite(pdf));
                SOFT_ASSERT(pdf > 0.f);

                return pdf * node.pdf(p, n, total_sphere, lo, light_mapping_, scene);
            }
        }
    }
}

void Tree::allocate_light_mapping(uint32_t num_lights) {
    if (num_lights_ != num_lights) {
        delete[] light_orders_;
        delete[] light_mapping_;

        light_mapping_ = new uint32_t[num_lights];
        light_orders_  = new uint32_t[num_lights];

        num_lights_ = num_lights;
    }
}

void Tree::allocate(uint32_t num_infinite_lights) {
    if (num_infinite_lights_ != num_infinite_lights) {
        delete[] infinite_light_powers_;

        infinite_light_powers_ = new float[num_infinite_lights];

        num_infinite_lights_ = num_infinite_lights;
    }
}

void Tree::allocate_nodes(uint32_t num_nodes) {
    if (num_nodes_ != num_nodes) {
        delete[] nodes_;

        nodes_ = new Node[num_nodes];

        num_nodes_ = num_nodes;
    }
}

Tree_builder::Tree_builder() : build_nodes_(nullptr), candidates_(nullptr) {}

Tree_builder::~Tree_builder() {
    delete[] candidates_;
    delete[] build_nodes_;
}

void Tree_builder::build(Tree& tree, Scene const& scene) {
    tree.allocate_light_mapping(scene.num_lights());

    light_order_ = 0;

    uint32_t lm = 0;

    for (uint32_t l = 0, len = scene.num_lights(); l < len; ++l) {
        if (!scene.light(l).is_finite(scene)) {
            tree.light_mapping_[lm++] = l;
        }
    }

    uint32_t const num_infinite_lights = lm;

    for (uint32_t l = 0, len = scene.num_lights(); l < len; ++l) {
        if (scene.light(l).is_finite(scene)) {
            tree.light_mapping_[lm++] = l;
        }
    }

    tree.allocate(num_infinite_lights);

    float infinite_total_power = 0.f;

    for (uint32_t i = 0; i < num_infinite_lights; ++i) {
        uint32_t const l = tree.light_mapping_[i];

        float const power = scene.light_power(l);

        tree.infinite_light_powers_[i] = power;

        tree.light_orders_[l] = light_order_++;

        infinite_total_power += power;
    }

    tree.infinite_end_ = light_order_;

    tree.infinite_light_distribution_.init(tree.infinite_light_powers_, num_infinite_lights);

    uint32_t const num_finite_lights = scene.num_lights() - num_infinite_lights;

    uint32_t infinite_depth_bias = 0;

    if (num_finite_lights > 0) {
        delete[] build_nodes_;

        uint32_t const num_nodes = 2 * num_finite_lights - 1;

        build_nodes_ = new Build_node[num_nodes];

        delete[] candidates_;

        if (num_finite_lights >= 2) {
            candidates_ = new Split_candidate[num_finite_lights - 1];
        } else {
            candidates_ = nullptr;
        }

        current_node_ = 1;

        uint32_t const num_total_lights = num_infinite_lights + num_finite_lights;

        split(tree, 0, num_infinite_lights, num_total_lights, scene);

        tree.allocate_nodes(current_node_);
        serialize(tree.nodes_);

        uint32_t max_splits = 0;
        build_nodes_[0].count_max_splits(0, build_nodes_, max_splits);

        if (num_infinite_lights > 0 && num_infinite_lights < Tree::Max_lights - 1) {
            if (uint32_t const left = Tree::Max_lights - max_splits; left < num_infinite_lights) {
                infinite_depth_bias = std::max(
                    uint32_t(std::ceil(std::log2(num_infinite_lights - left))), 1u);
            }
        }
    } else {
        tree.allocate_nodes(0);
    }

    tree.infinite_depth_bias_ = infinite_depth_bias;

    float const p0 = infinite_total_power;
    float const p1 = 0 == num_finite_lights ? 0.f : build_nodes_[0].power;

    float const pt = p0 + p1;

    float const infinite_weight = p0 / pt;

    tree.infinite_weight_ = infinite_weight;

    // This is because I'm afraid of the 1.f == random case
    tree.infinite_guard_ = 0 == num_finite_lights ? 1.1f : infinite_weight;
}

static void sort_lights(uint32_t* const lights, uint32_t begin, uint32_t end, uint32_t axis,
                        Scene const& scene) {
    std::sort(lights + begin, lights + end, [&scene, axis](uint32_t a, uint32_t b) noexcept {
        float3 const ac = scene.light_aabb(a).position();
        float3 const bc = scene.light_aabb(b).position();

        return ac[axis] < bc[axis];
    });
}

static void evaluate_splits(uint32_t* const lights, uint32_t begin, uint32_t end, uint32_t axis,
                            float aabb_surface_area, float cone_weight,
                            Tree_builder::Split_candidate* candidates, Scene const& scene) {
    sort_lights(lights, begin, end, axis, scene);

    for (uint32_t i = begin + 1, j = 0; i < end; ++i, ++j) {
        candidates[j].init(begin, end, i, aabb_surface_area, cone_weight, lights, scene);
    }

    uint32_t const len = end - begin;

    using SC = Tree_builder::Split_candidate;

    std::nth_element(candidates, candidates, candidates + len - 1,
                     [](SC const& a, SC const& b) noexcept { return a.weight < b.weight; });
}

static float cone_importance(float cos) {
    float const o = std::acos(cos);
    float const w = std::min(o, Pi);

    float const sin = std::sin(o);

    float const b = (Pi / 2.f) * (2.f * w * sin - std::cos(o - 2.f * w) - 2.f * o * sin + cos);

    return (2.f * Pi) * (1.f - cos) + b;
}

static float variance(uint32_t* const lights, uint32_t begin, uint32_t end, Scene const& scene) {
    float ap  = 0.f;
    float aps = 0.f;

    for (uint32_t i = begin, n = 0; i < end; ++i, ++n) {
        uint32_t const l = lights[i];

        float const p = scene.light_power(l);

        float const in = 1.f / float(n + 1);

        ap += (p - ap) * in;
        aps += (p * p - aps) * in;
    }

    return std::abs(aps - ap * ap);
}

uint32_t Tree_builder::split(Tree& tree, uint32_t node_id, uint32_t begin, uint32_t end,
                             Scene const& scene) {
    uint32_t* const lights = tree.light_mapping_;

    Build_node& node = build_nodes_[node_id];

    uint32_t const len = end - begin;

    if (len <= 4) {
        AABB bounds(AABB::empty());

        float4 cone(1.f);

        float total_power = 0.f;

        for (uint32_t i = begin; i < end; ++i) {
            uint32_t const l = lights[i];

            bounds.merge_assign(scene.light_aabb(l));

            cone = cone_union(cone, scene.light_cone(l));

            total_power += scene.light_power(l);

            tree.light_orders_[l] = light_order_++;
        }

        node.bounds            = bounds;
        node.cone              = cone;
        node.power             = total_power;
        node.variance          = variance(lights, begin, end, scene);
        node.middle            = 0;
        node.children_or_light = begin;
        node.num_lights        = len;

        return begin + len;
    }

    uint32_t const child0 = current_node_;

    current_node_ += 2;

    AABB bounds(AABB::empty());

    float4 cone(1.f);

    float total_power = 0.f;

    for (uint32_t i = begin; i < end; ++i) {
        uint32_t const l = lights[i];

        bounds.merge_assign(scene.light_aabb(l));

        cone = cone_union(cone, scene.light_cone(l));

        total_power += scene.light_power(l);
    }

    float const surface_area = bounds.surface_area();

    float const cone_weight = cone_importance(cone[3]);

    float3 const extent = bounds.extent();

    float const max_axis = max_component(extent);

    float3 weights;
    uint3  split_nodes;

    {
        evaluate_splits(lights, begin, end, 0, surface_area, cone_weight, candidates_, scene);

        float const reg = max_axis / extent[0];

        weights[0]     = reg * candidates_[0].weight;
        split_nodes[0] = candidates_[0].split_node;
    }

    {
        evaluate_splits(lights, begin, end, 1, surface_area, cone_weight, candidates_, scene);

        float const reg = max_axis / extent[1];

        weights[1]     = reg * candidates_[0].weight;
        split_nodes[1] = candidates_[0].split_node;
    }

    {
        evaluate_splits(lights, begin, end, 2, surface_area, cone_weight, candidates_, scene);

        float const reg = max_axis / extent[2];

        weights[2]     = reg * candidates_[0].weight;
        split_nodes[2] = candidates_[0].split_node;
    }

    uint32_t const axis = index_min_component(weights);

    sort_lights(lights, begin, end, axis, scene);

    uint32_t const split_node = split_nodes[axis];

    uint32_t const c0_end = split(tree, child0, begin, split_node, scene);
    uint32_t const c1_end = split(tree, child0 + 1, split_node, end, scene);

    node.bounds            = bounds;
    node.cone              = cone;
    node.power             = total_power;
    node.variance          = variance(lights, begin, end, scene);
    node.middle            = c0_end;
    node.children_or_light = child0;
    node.num_lights        = len;

    return c1_end;
}

Tree_builder::Split_candidate::Split_candidate() = default;

void Tree_builder::Split_candidate::init(uint32_t begin, uint32_t end, uint32_t split,
                                         float surface_area, float cone_weight,
                                         uint32_t const* const lights, Scene const& scene) {
    AABB a(AABB::empty());

    float4 cone_a(1.f);

    float power_a = 0.f;

    for (uint32_t i = begin; i < split; ++i) {
        uint32_t const l = lights[i];
        a.merge_assign(scene.light_aabb(l));

        cone_a = cone_union(cone_a, scene.light_cone(l));

        power_a += scene.light_power(l);
    }

    float const cone_weight_a = cone_importance(cone_a[3]);

    AABB b(AABB::empty());

    float4 cone_b(1.f);

    float power_b = 0.f;

    for (uint32_t i = split; i < end; ++i) {
        uint32_t const l = lights[i];
        b.merge_assign(scene.light_aabb(l));

        cone_b = cone_union(cone_b, scene.light_cone(l));

        power_b += scene.light_power(l);
    }

    float const cone_weight_b = cone_importance(cone_b[3]);

    split_node = split;

    weight = (((power_a * cone_weight_a * a.surface_area()) +
               (power_b * cone_weight_b * b.surface_area())) /
              (surface_area * cone_weight));
}

void Tree_builder::serialize(Tree::Node* nodes) {
    for (uint32_t i = 0, len = current_node_; i < len; ++i) {
        Build_node const& source = build_nodes_[i];

        Tree::Node& dest = nodes[i];

        AABB const& bounds = source.bounds;

        dest.center            = float4(bounds.position(), 0.5f * length(bounds.extent()));
        dest.cone              = source.cone;
        dest.power             = source.power;
        dest.variance          = source.variance;
        dest.middle            = source.middle;
        dest.children_or_light = source.children_or_light;
        dest.num_lights        = source.num_lights;
    }
}

}  // namespace scene::light
