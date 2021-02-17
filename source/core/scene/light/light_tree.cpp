#include "light_tree.hpp"
#include "base/math/distribution/distribution_1d.inl"
#include "base/memory/array.inl"
#include "scene/material/material_sample_helper.hpp"
#include "scene/scene.inl"
#include "scene/shape/shape.inl"
#include "scene/shape/triangle/triangle_mesh.hpp"

#include "base/debug/assert.hpp"

// More or less complete implementation of
// Importance Sampling of Many Lights with Adaptive Tree Splitting
// http://aconty.com/pdf/many-lights-hpg2018.pdf

namespace scene::light {

static inline float clamped_cos_sub(float cos_a, float cos_b, float sin_a, float sin_b) {
    float const angle = cos_a * cos_b + sin_a * sin_b;
    return (cos_a > cos_b) ? 1.f : angle;
}

static inline float clamped_sin_sub(float cos_a, float cos_b, float sin_a, float sin_b) {
    float const angle = sin_a * cos_b - sin_b * cos_a;
    return (cos_a > cos_b) ? 0.f : angle;
}

static inline float importance(float3_p p, float3_p n, float3_p center, float4_p cone, float radius,
                               float power, bool two_sided, bool total_sphere) {
    float3 const axis = center - p;

    float const il = rlength(axis);

    float3 const na = il * axis;
    float3 const da = cone.xyz();

    float const sin_cu   = std::min(il * radius, 1.f);
    float const cos_cone = cone[3];
    float const cos_a    = material::abs_reverse_dot(da, na, two_sided);
    float const cos_n    = dot(n, na);

    Simd3f const sa(float3(sin_cu, cos_cone, cos_a, cos_n));
    Simd3f const sb = max(simd::One - sa * sa, simd::Min_normal.v);
    Simd3f const sr = sqrt(sb);
    float3 const out(sr);

    float const cos_cu   = out[0];
    float const sin_cone = out[1];
    float const sin_a    = out[2];
    float const sin_n    = out[3];

    float const ta = clamped_cos_sub(cos_a, cos_cone, sin_a, sin_cone);
    float const tb = clamped_sin_sub(cos_a, cos_cone, sin_a, sin_cone);
    float const tc = clamped_cos_sub(ta, cos_cu, tb, sin_cu);
    float const tn = clamped_cos_sub(cos_n, cos_cu, sin_n, sin_cu);

    float const ra     = total_sphere ? 1.f : tn;
    float const rb     = std::max(tc, 0.f);
    float const id_min = std::min(2.f / radius, il);
    float const base   = power * (id_min * id_min);

    return std::max(ra * rb * base, material::Dot_min);
}

static inline float importance(float3_p p0, float3_p p1, float3_p dir, float3_p center,
                               float4_p cone, float radius, float power, bool two_sided) {
    float3 const axis = p0 - center;

    float3 const v0 = normalize(axis);
    float3 const v1 = normalize(p1 - center);

    float3 const o0  = v0;
    float3 const uv2 = cross(v0, v1);

    float const uv2l = length(uv2);

    if (uv2l < material::Dot_min) {
        return power;
    }

    float3 const v2 = uv2 / uv2l;
    float3 const o1 = cross(o0, v2);

    float3 const da = cone.xyz();

    float const cos_a0 = material::clamp_abs_dot(o0, da, two_sided);
    float const cos_a1 = material::clamp_abs_dot(o1, da, two_sided);

    float const cos_phi = cos_a0 / std::sqrt(cos_a0 * cos_a0 + cos_a1 * cos_a1);
    float const sin_phi = std::sqrt(std::max(1.f - cos_phi * cos_phi, 0.f));

    float const delta = -dot(axis, dir);

    float3 const closest_point = p0 + delta * dir;

    float const l = std::max(distance(closest_point, center), material::Dot_min);

    float const sin_cu = std::min(radius / l, 1.f);
    float const cos_cu = std::sqrt(1.f - sin_cu * sin_cu);

    float cos_theta_min;

    if (cos_a1 < 0.f || dot(v0, v1) > cos_phi) {
        float const b_max = std::max(material::abs_dot(v0, da, two_sided),
                                     material::abs_dot(v1, da, two_sided));

        cos_theta_min = std::min(b_max, 1.f);
    } else {
        float3 const v = cos_phi * o0 + sin_phi * o1;

        cos_theta_min = clamp(material::abs_dot(v, da, two_sided), -1.f, 1.f);
    }

    float const cos_cone = cone[3];
    float const sin_cone = std::sqrt(1.f - cos_cone * cos_cone);

    float const cos_a = cos_theta_min;
    float const sin_a = std::sqrt(1.f - cos_a * cos_a);

    float const d0 = clamped_cos_sub(cos_a, cos_cone, sin_a, sin_cone);
    float const d1 = clamped_sin_sub(cos_a, cos_cone, sin_a, sin_cone);
    float const d2 = std::max(clamped_cos_sub(d0, cos_cu, d1, sin_cu), 0.f);

    return std::max((d2 * power) / l, material::Dot_min);
}

template <typename Set>
static float light_weight(float3_p p, float3_p n, bool total_sphere, uint32_t light,
                          Set const& set) {
    bool const   two_sided = set.light_two_sided(light);
    AABB const   aabb      = set.light_aabb(light);
    float3 const center    = aabb.position();
    float4 const cone      = set.light_cone(light);
    float const  radius    = aabb.cached_radius();
    float const  power     = set.light_power(light);

    return importance(p, n, center, cone, radius, power, two_sided, total_sphere);
}

static float light_weight(float3_p p0, float3_p p1, float3_p dir, uint32_t light,
                          Scene const& scene) {
    bool const   two_sided = scene.light_two_sided(light);
    AABB const   aabb      = scene.light_aabb(light);
    float3 const center    = aabb.position();
    float4 const cone      = scene.light_cone(light);
    float const  radius    = aabb.cached_radius();
    float const  power     = scene.light_power(light);

    return importance(p0, p1, center, dir, cone, radius, power, two_sided);
}

struct Build_node {
    bool has_children() const {
        return middle > 0;
    }

    void count_max_splits(uint32_t depth, Build_node* nodes, uint32_t& splits) const {
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

    AABB bounds;

    float4 cone;

    float power;
    float variance;

    uint32_t middle;
    uint32_t children_or_light;
    uint32_t num_lights;
};

inline float Node::weight(float3_p p, float3_p n, bool total_sphere) const {
    float const r            = center[3];
    bool const  is_two_sided = 1 == two_sided;

    return importance(p, n, center.xyz(), cone, r, power, is_two_sided, total_sphere);
}

inline float Node::weight(float3_p p0, float3_p p1, float3_p dir) const {
    float const r            = center[3];
    bool const  is_two_sided = 1 == two_sided;

    return importance(p0, p1, center.xyz(), dir, cone, r, power, is_two_sided);
}

inline bool Node::split(float3_p p) const {
    float const r = center[3];
    float const d = std::min(distance(p, center.xyz()), 1.e6f);

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

    float const s2 = std::max(ve * vg + ve * eg2 + ee * ee * vg, 0.f);
    float const ns = 1.f / (1.f + std::sqrt(s2));
    // float const ns = std::pow(1.f / (1.f + std::sqrt(s2)), 1.f / 4.f);

    SOFT_ASSERT(std::isfinite(ns));

    return ns <= Tree::splitting_threshold_;
}

inline bool Node::split(float3_p p0, float3_p dir) const {
    float3 const axis = p0 - center.xyz();

    float const delta = -dot(axis, dir);

    float3 const closest_point = p0 + delta * dir;

    return split(closest_point);
}

template <typename Set>
Light_pick Node::random_light(float3_p p, float3_p n, bool total_sphere, float random,
                              UInts light_mapping, Set const& set) const {
    if (1 == num_lights) {
        return {light_mapping[children_or_light], 1.f};
    }

    float weights[4] = {0.f, 0.f, 0.f, 0.f};

    for (uint32_t i = 0, len = num_lights; i < len; ++i) {
        weights[i] = light_weight(p, n, total_sphere, light_mapping[children_or_light + i], set);
    }

    auto const l = distribution_sample_discrete<4>(weights, num_lights, random);

    return {light_mapping[children_or_light + l.offset], l.pdf};
}

inline Light_pick Node::random_light(float3_p p0, float3_p p1, float3_p dir, float random,
                                     UInts light_mapping, Scene const& scene) const {
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

template <typename Set>
float Node::pdf(float3_p p, float3_p n, bool total_sphere, uint32_t id,
                uint32_t const* const light_mapping, Set const& set) const {
    if (1 == num_lights) {
        return 1.f;
    }

    float weights[4] = {0.f, 0.f, 0.f, 0.f};

    for (uint32_t i = 0, len = num_lights; i < len; ++i) {
        weights[i] = light_weight(p, n, total_sphere, light_mapping[children_or_light + i], set);
    }

    return distribution_pdf<4>(weights, id - children_or_light);
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
      node_middles_(nullptr),
      light_orders_(nullptr),
      light_mapping_(nullptr),
      infinite_light_powers_(nullptr) {}

Tree::~Tree() {
    delete[] infinite_light_powers_;
    delete[] light_mapping_;
    delete[] light_orders_;
    delete[] node_middles_;
    delete[] nodes_;
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

    if (0 == num_nodes_) {
        return;
    }

    float const pdf = 1.f - ip;

    Traversal_stack stack;

    Traversal_stack::Node t{pdf, (random - ip) / pdf, 0, split ? depth_bias : Max_split_depth};

    stack.push(t);

    while (!stack.empty()) {
        Node const& node = nodes_[t.node];

        bool const do_split = t.depth < Max_split_depth && node.split(p);

        if (1 == node.has_children) {
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

                t.depth = Max_split_depth;
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

                lights.push_back({pick.offset, pick.pdf * t.pdf});
            }

            t = stack.pop();
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

    if (0 == num_nodes_) {
        return;
    }

    float3 const dir = normalize(p0 - p1);

    float const pdf = 1.f - ip;

    Traversal_stack stack;

    Traversal_stack::Node t{pdf, (random - ip) / pdf, 0, split ? depth_bias : Max_split_depth};

    stack.push(t);

    while (!stack.empty()) {
        Node const& node = nodes_[t.node];

        bool const do_split = t.depth < Max_split_depth && node.split(p0, dir);

        if (1 == node.has_children) {
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

                t.depth = Max_split_depth;
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

                lights.push_back({pick.offset, pick.pdf * t.pdf});
            }

            t = stack.pop();
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

    if (0 == num_nodes_) {
        return 0.f;
    }

    float const ip = split_infinite ? 0.f : infinite_weight_;

    float pdf = 1.f - ip;

    SOFT_ASSERT(pdf > 0.f);

    for (uint32_t nid = 0, depth = split ? infinite_depth_bias_ : Max_split_depth;; ++depth) {
        Node const& node = nodes_[nid];

        bool const do_split = depth < Max_split_depth && node.split(p);

        if (1 == node.has_children) {
            uint32_t const c0 = node.children_or_light;
            uint32_t const c1 = c0 + 1;

            uint32_t const middle = node_middles_[nid];

            if (do_split) {
                if (lo < middle) {
                    nid = c0;
                } else {
                    nid = c1;
                }
            } else {
                float const p0 = nodes_[c0].weight(p, n, total_sphere);
                float const p1 = nodes_[c1].weight(p, n, total_sphere);
                float const pt = p0 + p1;

                SOFT_ASSERT(pt > 0.f);

                if (lo < middle) {
                    nid = c0;
                    pdf *= p0 / pt;
                } else {
                    nid = c1;
                    pdf *= p1 / pt;
                }

                depth = Max_split_depth;
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

void Tree::allocate_nodes(uint32_t num_nodes) {
    if (num_nodes_ != num_nodes) {
        delete[] nodes_;
        delete[] node_middles_;

        nodes_        = new Node[num_nodes];
        node_middles_ = new uint32_t[num_nodes];

        num_nodes_ = num_nodes;
    }
}

void Tree::allocate(uint32_t num_infinite_lights) {
    if (num_infinite_lights_ != num_infinite_lights) {
        delete[] infinite_light_powers_;

        infinite_light_powers_ = new float[num_infinite_lights];

        num_infinite_lights_ = num_infinite_lights;
    }
}

Primitive_tree::Primitive_tree()
    : num_lights_(0),
      num_nodes_(0),
      nodes_(nullptr),
      node_middles_(nullptr),
      distributions_(nullptr),
      light_orders_(nullptr),
      light_mapping_(nullptr) {}

Primitive_tree::~Primitive_tree() {
    delete[] light_mapping_;
    delete[] light_orders_;
    delete[] distributions_;
    delete[] node_middles_;
    delete[] nodes_;
}

Light_pick Primitive_tree::random_light(float3_p p, float3_p n, bool total_sphere,
                                        float random) const {
    float pdf = 1.f;

    for (uint32_t nid = 0;;) {
        Node const& node = nodes_[nid];

        if (1 == node.has_children) {
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
            SOFT_ASSERT(std::isfinite(pdf));
            SOFT_ASSERT(pdf > 0.f);

            auto const pick = distributions_[nid].sample_discrete(random);

            return {light_mapping_[node.children_or_light + pick.offset], pick.pdf * pdf};
        }
    }
}

float Primitive_tree::pdf(float3_p p, float3_p n, bool total_sphere, uint32_t id) const {
    uint32_t const lo = light_orders_[id];

    float pdf = 1.f;

    for (uint32_t nid = 0;;) {
        Node const& node = nodes_[nid];

        uint32_t const middle = node_middles_[nid];

        if (middle > 0) {
            uint32_t const c0 = node.children_or_light;
            uint32_t const c1 = c0 + 1;

            float const p0 = nodes_[c0].weight(p, n, total_sphere);
            float const p1 = nodes_[c1].weight(p, n, total_sphere);

            float const pt = p0 + p1;

            SOFT_ASSERT(pt > 0.f);

            if (lo < middle) {
                nid = c0;
                pdf *= p0 / pt;
            } else {
                nid = c1;
                pdf *= p1 / pt;
            }
        } else {
            SOFT_ASSERT(std::isfinite(pdf));
            SOFT_ASSERT(pdf > 0.f);

            return pdf * distributions_[nid].pdf(lo - node.children_or_light);
        }
    }
}

void Primitive_tree::allocate_light_mapping(uint32_t num_lights) {
    if (num_lights_ != num_lights) {
        delete[] light_orders_;
        delete[] light_mapping_;

        light_mapping_ = new uint32_t[num_lights];
        light_orders_  = new uint32_t[num_lights];

        num_lights_ = num_lights;
    }
}

void Primitive_tree::allocate_nodes(uint32_t num_nodes) {
    if (num_nodes_ != num_nodes) {
        delete[] distributions_;
        delete[] nodes_;
        delete[] node_middles_;

        nodes_         = new Node[num_nodes];
        node_middles_  = new uint32_t[num_nodes];
        distributions_ = new Distribution_1D[num_nodes];

        num_nodes_ = num_nodes;
    }
}

}  // namespace scene::light
