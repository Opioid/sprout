#include "light_tree_builder.hpp"
#include "base/math/cone.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/memory/array.inl"
#include "base/memory/buffer.hpp"
#include "base/thread/thread_pool.hpp"
#include "light_tree.hpp"
#include "scene/scene.inl"
#include "scene/shape/shape.inl"
#include "scene/shape/triangle/triangle_mesh.hpp"

#include "base/debug/assert.hpp"

// More or less complete implementation of
// Importance Sampling of Many Lights with Adaptive Tree Splitting
// http://aconty.com/pdf/many-lights-hpg2018.pdf

namespace scene::light {

static uint32_t constexpr Scene_sweep_threshold = 128;
static uint32_t constexpr Part_sweep_threshold  = 32;
static uint32_t constexpr Num_slices            = 16;

using UInts = uint32_t const* const;

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

    bool two_sided;
};

template <typename Set>
static void sort_lights(uint32_t* const lights, uint32_t begin, uint32_t end, uint32_t axis,
                        Set const& set) {
    std::sort(lights + begin, lights + end, [&set, axis](uint32_t a, uint32_t b) noexcept {
        float3 const ac = set.light_aabb(a).position();
        float3 const bc = set.light_aabb(b).position();

        return ac[axis] < bc[axis];
    });
}

static float cone_cost(float cos) {
    float const o = std::acos(cos);
    float const w = std::min(o + (Pi / 2.f), Pi);

    float const sin = std::sin(o);

    float const b = (Pi / 2.f) * (2.f * w * sin - std::cos(o - 2.f * w) - 2.f * o * sin + cos);

    return (2.f * Pi) * (1.f - cos) + b;
}

template <typename Set>
static float variance(uint32_t* const lights, uint32_t begin, uint32_t end, Set const& set) {
    float ap  = 0.f;
    float aps = 0.f;

    for (uint32_t i = begin, n = 0; i < end; ++i, ++n) {
        uint32_t const l = lights[i];

        float const p  = set.light_power(l);
        float const in = 1.f / float(n + 1);

        ap += (p - ap) * in;
        aps += (p * p - aps) * in;
    }

    return std::abs(aps - ap * ap);
}

struct Split_candidate {
    using Part = shape::triangle::Part;

    Split_candidate();

    void init(float3_p p, uint32_t axis);

    void evaluate(uint32_t begin, uint32_t end, UInts lights, AABB const& bounds, float cone_weight,
                  Scene const& scene);

    void evaluate(uint32_t begin, uint32_t end, UInts lights, AABB const& bounds, float cone_weight,
                  Part const& part);

    bool behind(float const* point) const {
        return point[axis_] < d_;
    }

    AABB aabb_0_;
    AABB aabb_1_;

    float4 cone_0_;
    float4 cone_1_;

    float power_0_;
    float power_1_;

    float d_;
    float cost_;

    uint32_t axis_;

    bool two_sided_0_;
    bool two_sided_1_;

    bool exhausted_;
};

Split_candidate::Split_candidate() = default;

void Split_candidate::init(float3_p p, uint32_t axis) {
    d_ = p[axis];

    axis_ = axis;
}

void Split_candidate::evaluate(uint32_t begin, uint32_t end, UInts lights, AABB const& bounds,
                               float cone_weight, Scene const& scene) {
    uint32_t num_side_0 = 0;
    uint32_t num_side_1 = 0;

    Simd_AABB box_0(Empty_AABB);
    Simd_AABB box_1(Empty_AABB);

    float4 cone_0(1.f);
    float4 cone_1(1.f);

    bool two_sided_0 = false;
    bool two_sided_1 = false;

    float power_0(0.f);
    float power_1(0.f);

    for (uint32_t i = begin; i < end; ++i) {
        uint32_t const l = lights[i];

        AABB const box(scene.light_aabb(l));

        float4 const cone = scene.light_cone(l);

        bool const two_sided = scene.light_two_sided(l);

        float const power = scene.light_power(l);

        if (behind(box.max().v)) {
            ++num_side_0;

            box_0.merge_assign(box);
            cone_0 = cone::merge(cone_0, cone);
            two_sided_0 |= two_sided;
            power_0 += power;
        } else {
            ++num_side_1;

            box_1.merge_assign(box);
            cone_1 = cone::merge(cone_1, cone);
            two_sided_1 |= two_sided;
            power_1 += power;
        }
    }

    float3 const extent = bounds.extent();

    float const reg          = max_component(extent) / extent[axis_];
    float const surface_area = bounds.surface_area();

    aabb_0_ = AABB(box_0);
    aabb_1_ = AABB(box_1);

    cone_0_ = cone_0;
    cone_1_ = cone_1;

    power_0_ = power_0;
    power_1_ = power_1;

    two_sided_0_ = two_sided_0;
    two_sided_1_ = two_sided_1;

    if (bool const empty_side = 0 == num_side_0 || 0 == num_side_1; empty_side) {
        cost_ = 2.f * reg * (power_0 + power_1) * (4.f * Pi) * surface_area * float(end - begin);

        exhausted_ = true;
    } else {
        float const cone_weight_a = cone_cost(cone_0[3]) * (two_sided_0 ? 2.f : 1.f);
        float const cone_weight_b = cone_cost(cone_1[3]) * (two_sided_1 ? 2.f : 1.f);
        ;

        float const surface_area_a = AABB(box_0).surface_area();
        float const surface_area_b = AABB(box_1).surface_area();

        cost_ = reg * (((power_0 * cone_weight_a * surface_area_a) +
                        (power_1 * cone_weight_b * surface_area_b)) /
                       (surface_area * cone_weight));

        exhausted_ = false;
    }
}

void Split_candidate::evaluate(uint32_t begin, uint32_t end, UInts lights, AABB const& bounds,
                               float cone_weight, Part const& part) {
    uint32_t num_side_0 = 0;
    uint32_t num_side_1 = 0;

    Simd_AABB box_0(Empty_AABB);
    Simd_AABB box_1(Empty_AABB);

    float power_0(0.f);
    float power_1(0.f);

    float3 dominant_axis_0(0.f);
    float3 dominant_axis_1(0.f);

    float const a = 1.f / part.distribution.integral();

    for (uint32_t i = begin; i < end; ++i) {
        uint32_t const l = lights[i];

        AABB const box(part.light_aabb(l));

        float3 const n = part.light_cone(l).xyz();

        float const power = part.light_power(l);

        if (behind(box.max().v)) {
            ++num_side_0;

            box_0.merge_assign(box);
            power_0 += power;

            dominant_axis_0 += a * power * n;
        } else {
            ++num_side_1;

            box_1.merge_assign(box);
            power_1 += power;

            dominant_axis_1 += a * power * n;
        }
    }

    dominant_axis_0 = normalize(dominant_axis_0);
    dominant_axis_1 = normalize(dominant_axis_1);

    float angle_0 = 0.f;
    float angle_1 = 0.f;

    for (uint32_t i = begin; i < end; ++i) {
        uint32_t const l = lights[i];

        AABB const box(part.light_aabb(l));

        float3 const n = part.light_cone(l).xyz();

        if (behind(box.max().v)) {
            float const c = dot(dominant_axis_0, n);

            angle_0 = std::max(angle_0, std::acos(c));
        } else {
            float const c = dot(dominant_axis_1, n);

            angle_1 = std::max(angle_1, std::acos(c));
        }
    }

    float4 const cone_0 = float4(dominant_axis_0, std::cos(angle_0));
    float4 const cone_1 = float4(dominant_axis_1, std::cos(angle_1));

    float3 const extent = bounds.extent();

    float const reg          = max_component(extent) / extent[axis_];
    float const surface_area = bounds.surface_area();

    aabb_0_ = AABB(box_0);
    aabb_1_ = AABB(box_1);

    cone_0_ = cone_0;
    cone_1_ = cone_1;

    power_0_ = power_0;
    power_1_ = power_1;

    if (bool const empty_side = 0 == num_side_0 || 0 == num_side_1; empty_side) {
        cost_ = 2.f * reg * (power_0 + power_1) * (4.f * Pi) * surface_area * float(end - begin);

        exhausted_ = true;
    } else {
        float const cone_weight_a = cone_cost(cone_0[3]);
        float const cone_weight_b = cone_cost(cone_1[3]);

        float const surface_area_a = AABB(box_0).surface_area();
        float const surface_area_b = AABB(box_1).surface_area();

        cost_ = reg * (((power_0 * cone_weight_a * surface_area_a) +
                        (power_1 * cone_weight_b * surface_area_b)) /
                       (surface_area * cone_weight));

        exhausted_ = false;
    }
}

template <typename Set>
static Split_candidate evaluate_splits(uint32_t* const lights, uint32_t begin, uint32_t end,
                                       AABB const& bounds, float cone_weight,
                                       uint32_t sweep_threshold, Split_candidate* candidates,
                                       Set const& set, Threads& threads) {
    static uint32_t constexpr X = 0;
    static uint32_t constexpr Y = 1;
    static uint32_t constexpr Z = 2;

    uint32_t const len = end - begin;

    uint32_t num_candidates = 0;

    if (len < sweep_threshold) {
        for (uint32_t i = begin, back = end; i < back; ++i) {
            uint32_t const l = lights[i];

            float3 const max = set.light_aabb(l).max();

            candidates[num_candidates++].init(max, X);
            candidates[num_candidates++].init(max, Y);
            candidates[num_candidates++].init(max, Z);
        }
    } else {
        float3 const position = bounds.position();
        float3 const extent   = bounds.extent();
        float3 const min      = bounds.min();

        uint32_t const la = index_max_component(extent);

        float const step = (extent[la]) / float(Num_slices);

        for (uint32_t a = 0; a < 3; ++a) {
            float const extent_a = extent[a];

            uint32_t const num_steps = uint32_t(std::ceil(extent_a / step));

            float const step_a = extent_a / float(num_steps);

            for (uint32_t i = 1; i < num_steps; ++i) {
                float const fi = float(i);

                float3 slice = position;
                slice[a]     = min[a] + fi * step_a;

                candidates[num_candidates++].init(slice, a);
            }
        }
    }

    if (len * num_candidates > 1024) {
        threads.run_range(
            [candidates, lights, begin, end, &bounds, cone_weight, &set](
                uint32_t /*id*/, int32_t sc_begin, int32_t sc_end) noexcept {
                for (int32_t i = sc_begin; i < sc_end; ++i) {
                    candidates[uint32_t(i)].evaluate(begin, end, lights, bounds, cone_weight, set);
                }
            },
            0, int32_t(num_candidates));
    } else {
        for (uint32_t i = 0; i < num_candidates; ++i) {
            candidates[i].evaluate(begin, end, lights, bounds, cone_weight, set);
        }
    }

    float min_cost = candidates[0].cost_;

    uint32_t sc = 0;
    for (uint32_t i = 1; i < num_candidates; ++i) {
        if (float const cost = candidates[i].cost_; cost < min_cost) {
            sc = i;

            min_cost = cost;
        }
    }

    return candidates[sc];
}

Tree_builder::Tree_builder()
    : build_nodes_capacity_(0),
      candidates_capacity_(0),
      build_nodes_(nullptr),
      candidates_(nullptr) {}

Tree_builder::~Tree_builder() {
    delete[] candidates_;
    delete[] build_nodes_;
}

void Tree_builder::build(Tree& tree, Scene const& scene, Threads& threads) {
    uint32_t const num_lights = scene.num_lights();

    tree.allocate_light_mapping(num_lights);

    light_order_ = 0;

    uint32_t lm = 0;

    for (uint32_t l = 0; l < num_lights; ++l) {
        if (!scene.light(l).is_finite(scene)) {
            tree.light_mapping_[lm++] = l;
        }
    }

    uint32_t const num_infinite_lights = lm;

    for (uint32_t l = 0; l < num_lights; ++l) {
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

    uint32_t const num_finite_lights = num_lights - num_infinite_lights;

    uint32_t infinite_depth_bias = 0;

    if (num_finite_lights > 0) {
        allocate(num_finite_lights, Scene_sweep_threshold);

        current_node_ = 1;

        uint32_t const num_total_lights = num_infinite_lights + num_finite_lights;

        Simd_AABB tbounds(Empty_AABB);
        float4    cone(1.f);
        bool      two_sided = false;
        float     total_power(0.f);

        for (uint32_t i = 0; i < num_finite_lights; ++i) {
            uint32_t const l = tree.light_mapping_[i];

            tbounds.merge_assign(scene.light_aabb(l));
            cone = cone::merge(cone, scene.light_cone(l));
            two_sided |= scene.light_two_sided(l);
            total_power += scene.light_power(l);
        }

        AABB const bounds(tbounds);

        split(tree, 0, num_infinite_lights, num_total_lights, bounds, cone, two_sided, total_power,
              scene, threads);

        tree.allocate_nodes(current_node_);
        serialize(tree.nodes_, tree.node_middles_);

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

    float const infinite_weight = 0 == num_lights ? 0.f : p0 / pt;

    tree.infinite_weight_ = infinite_weight;

    // This is because I'm afraid of the 1.f == random case
    tree.infinite_guard_ = 0 == num_finite_lights ? (0 == num_infinite_lights ? 0.f : 1.1f)
                                                  : infinite_weight;
}

void Tree_builder::build(Primitive_tree& tree, Part const& part, bool two_sided, Threads& threads) {
    uint32_t const num_finite_lights = part.num_triangles;

    tree.allocate_light_mapping(num_finite_lights);

    light_order_ = 0;

    uint32_t lm = 0;

    for (uint32_t l = 0, len = part.num_triangles; l < len; ++l) {
        tree.light_mapping_[lm++] = l;
    }

    allocate(num_finite_lights, Part_sweep_threshold);

    current_node_ = 1;

    // In this case the surface area of the shape part
    float const total_power = part.distribution.integral();

    split(tree, 0, 0, num_finite_lights, std::max(num_finite_lights / 32, 8u), part.aabb, part.cone,
          total_power, part, two_sided, threads);

    tree.allocate_nodes(current_node_);

    serialize(tree, part);
}

void Tree_builder::allocate(uint32_t num_lights, uint32_t sweep_threshold) {
    uint32_t const num_nodes = 2 * num_lights - 1;

    if (num_nodes > build_nodes_capacity_) {
        delete[] build_nodes_;

        build_nodes_ = new Build_node[num_nodes];

        build_nodes_capacity_ = num_nodes;
    }

    uint32_t const num_slices = std::min(num_lights, sweep_threshold);

    uint32_t const num_candidates = num_slices >= 2 ? num_slices * 3 : 0;

    if (num_candidates > candidates_capacity_) {
        delete[] candidates_;

        candidates_ = new Split_candidate[num_candidates];

        candidates_capacity_ = num_candidates;
    }
}

uint32_t Tree_builder::split(Tree& tree, uint32_t node_id, uint32_t begin, uint32_t end,
                             AABB const& bounds, float4_p cone, bool two_sided, float total_power,
                             Scene const& scene, Threads& threads) {
    uint32_t* const lights = tree.light_mapping_;

    Build_node& node = build_nodes_[node_id];

    uint32_t const len = end - begin;

    if (len <= 4) {
        bool two_sided = false;

        for (uint32_t i = begin; i < end; ++i) {
            uint32_t const l = lights[i];

            tree.light_orders_[l] = light_order_++;

            two_sided |= scene.light_two_sided(l);
        }

        node.bounds            = bounds;
        node.cone              = cone;
        node.power             = total_power;
        node.variance          = variance(lights, begin, end, scene);
        node.middle            = 0;
        node.children_or_light = begin;
        node.num_lights        = len;
        node.two_sided         = two_sided;

        return begin + len;
    }

    uint32_t const child0 = current_node_;

    current_node_ += 2;

    float const cone_weight = cone_cost(cone[3]);

    Split_candidate const sc = evaluate_splits(lights, begin, end, bounds, cone_weight,
                                               Scene_sweep_threshold, candidates_, scene, threads);

    SOFT_ASSERT(!sc.exhausted_);

    uint32_t const split_node = std::partition(lights + begin, lights + end,
                                               [&sc, &scene](uint32_t l) {
                                                   float3 const max = scene.light_aabb(l).max();
                                                   return sc.behind(max.v);
                                               }) -
                                lights;

    uint32_t const c0_end = split(tree, child0, begin, split_node, sc.aabb_0_, sc.cone_0_,
                                  sc.two_sided_0_, sc.power_0_, scene, threads);
    uint32_t const c1_end = split(tree, child0 + 1, split_node, end, sc.aabb_1_, sc.cone_1_,
                                  sc.two_sided_1_, sc.power_1_, scene, threads);

    node.bounds            = bounds;
    node.cone              = cone;
    node.power             = total_power;
    node.variance          = variance(lights, begin, end, scene);
    node.middle            = c0_end;
    node.children_or_light = child0;
    node.num_lights        = len;
    node.two_sided         = two_sided;

    return c1_end;
}

uint32_t Tree_builder::split(Primitive_tree& tree, uint32_t node_id, uint32_t begin, uint32_t end,
                             uint32_t max_primitives, AABB const& bounds, float4_p cone,
                             float total_power, Part const& part, bool two_sided,
                             Threads& threads) {
    uint32_t* const lights = tree.light_mapping_;

    Build_node& node = build_nodes_[node_id];

    uint32_t const len = end - begin;

    if (len <= max_primitives || cone[3] > 0.9f) {
        return assign(node, tree, begin, end, bounds, cone, total_power, part, two_sided);
    }

    uint32_t const child0 = current_node_;

    float const cone_weight = cone_cost(cone[3]);

    Split_candidate const sc = evaluate_splits(lights, begin, end, bounds, cone_weight,
                                               Part_sweep_threshold, candidates_, part, threads);

    if (sc.exhausted_) {
        return assign(node, tree, begin, end, bounds, cone, total_power, part, two_sided);
    }

    uint32_t const split_node = std::partition(lights + begin, lights + end,
                                               [&sc, &part](uint32_t l) {
                                                   float3 const max = part.light_aabb(l).max();
                                                   return sc.behind(max.v);
                                               }) -
                                lights;

    current_node_ += 2;

    uint32_t const c0_end = split(tree, child0, begin, split_node, max_primitives, sc.aabb_0_,
                                  sc.cone_0_, sc.power_0_, part, two_sided, threads);
    uint32_t const c1_end = split(tree, child0 + 1, split_node, end, max_primitives, sc.aabb_1_,
                                  sc.cone_1_, sc.power_1_, part, two_sided, threads);

    node.bounds            = bounds;
    node.cone              = cone;
    node.power             = total_power;
    node.variance          = variance(lights, begin, end, part);
    node.middle            = c0_end;
    node.children_or_light = child0;
    node.num_lights        = len;
    node.two_sided         = two_sided;

    return c1_end;
}

uint32_t Tree_builder::assign(Build_node& node, Primitive_tree& tree, uint32_t begin, uint32_t end,
                              AABB const& bounds, float4_p cone, float total_power,
                              Part const& part, bool two_sided) {
    uint32_t* const lights = tree.light_mapping_;

    uint32_t const len = end - begin;

    for (uint32_t i = begin; i < end; ++i) {
        uint32_t const l = lights[i];

        tree.light_orders_[l] = light_order_++;
    }

    node.bounds            = bounds;
    node.cone              = cone;
    node.power             = total_power;
    node.variance          = variance(lights, begin, end, part);
    node.middle            = 0;
    node.children_or_light = begin;
    node.num_lights        = len;
    node.two_sided         = two_sided;

    return begin + len;
}

void Tree_builder::serialize(Node* nodes, uint32_t* node_middles) {
    for (uint32_t i = 0, len = current_node_; i < len; ++i) {
        Build_node const& source = build_nodes_[i];

        Node& dest = nodes[i];

        AABB const& bounds = source.bounds;

        dest.center            = float4(bounds.position(), 0.5f * length(bounds.extent()));
        dest.cone              = source.cone;
        dest.power             = source.power;
        dest.variance          = source.variance;
        dest.has_children      = source.has_children() ? 1 : 0;
        dest.children_or_light = source.children_or_light;
        dest.num_lights        = source.num_lights;
        dest.two_sided         = source.two_sided ? 1 : 0;

        node_middles[i] = source.middle;
    }
}

void Tree_builder::serialize(Primitive_tree& tree, Part const& part) {
    for (uint32_t i = 0, len = current_node_; i < len; ++i) {
        Build_node const& source = build_nodes_[i];

        Node& dest = tree.nodes_[i];

        AABB const& bounds = source.bounds;

        dest.center            = float4(bounds.position(), 0.5f * length(bounds.extent()));
        dest.cone              = source.cone;
        dest.power             = source.power;
        dest.variance          = source.variance;
        dest.has_children      = source.has_children() ? 1 : 0;
        dest.children_or_light = source.children_or_light;
        dest.num_lights        = source.num_lights;
        dest.two_sided         = source.two_sided ? 1 : 0;

        tree.node_middles_[i] = source.middle;

        if (!source.has_children()) {
            uint32_t const first = source.children_or_light;
            uint32_t const num   = source.num_lights;

            memory::Buffer<float> powers(num);

            for (uint32_t t = 0; t < num; ++t) {
                powers[t] = part.light_power(tree.light_mapping_[t + first]);
            }

            tree.distributions_[i].init(powers, num);
        }
    }
}

}  // namespace scene::light
