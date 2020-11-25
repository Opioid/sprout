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

static float cone_importance(float cos) {
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
    Split_candidate();

    void init(uint32_t begin, uint32_t end, uint32_t split);

    template <typename Set>
    void evaluate(UInts lights, Set const& set);

    uint32_t begin_;
    uint32_t end_;

    uint32_t split_node;

    float weight;
};

Split_candidate::Split_candidate() = default;

void Split_candidate::init(uint32_t begin, uint32_t end, uint32_t split) {
    begin_     = begin;
    end_       = end;
    split_node = split;
}

template <typename Set>
void Split_candidate::evaluate(UInts lights, Set const& set) {
    uint32_t const begin = begin_;
    uint32_t const end   = end_;
    uint32_t const split = split_node;

    Simd_AABB box_a(Empty_AABB);
    float4    cone_a(1.f);
    float     power_a(0.f);

    for (uint32_t i = begin; i < split; ++i) {
        uint32_t const l = lights[i];

        box_a.merge_assign(set.light_aabb(l));
        cone_a = cone::merge(cone_a, set.light_cone(l));
        power_a += set.light_power(l);
    }

    float const cone_weight_a = cone_importance(cone_a[3]);

    Simd_AABB box_b(Empty_AABB);
    float4    cone_b(1.f);
    float     power_b(0.f);

    for (uint32_t i = split; i < end; ++i) {
        uint32_t const l = lights[i];

        box_b.merge_assign(set.light_aabb(l));
        cone_b = cone::merge(cone_b, set.light_cone(l));
        power_b += set.light_power(l);
    }

    float const cone_weight_b = cone_importance(cone_b[3]);

    split_node = split;

    weight = (power_a * cone_weight_a * AABB(box_a).surface_area()) +
             (power_b * cone_weight_b * AABB(box_b).surface_area());
}

template <typename Set>
static uint32_t evaluate_splits(uint32_t* const lights, uint32_t begin, uint32_t end,
                                uint32_t stride, uint32_t axis, Split_candidate* candidates,
                                Set const& set, Threads& threads) {
    sort_lights(lights, begin, end, axis, set);

    uint32_t num_candidates = 0;

    for (uint32_t i = begin + stride; i < end; i += stride, ++num_candidates) {
        candidates[num_candidates].init(begin, end, std::min(i, end - 1));
    }

    if (uint32_t len = end - begin; len * num_candidates > 1024) {
        threads.run_range(
            [candidates, lights, &set](uint32_t /*id*/, int32_t sc_begin, int32_t sc_end) noexcept {
                for (int32_t i = sc_begin; i < sc_end; ++i) {
                    candidates[uint32_t(i)].evaluate(lights, set);
                }
            },
            0, int32_t(num_candidates));
    } else {
        for (uint32_t i = 0; i < num_candidates; ++i) {
            candidates[i].evaluate(lights, set);
        }
    }

    using SC = Split_candidate;

    std::nth_element(candidates, candidates, candidates + num_candidates,
                     [](SC const& a, SC const& b) noexcept { return a.weight < b.weight; });

    return 0;

    //    float min_cost = candidates[0].weight;

    //    uint32_t sc = 0;
    //    for (uint32_t i = 1; i < num_candidates; ++i) {
    //        if (float const cost = candidates[i].weight; cost < min_cost) {
    //            sc = i;

    //            min_cost = cost;
    //        }
    //    }

    //    return sc;
}

Tree_builder::Tree_builder() : build_nodes_(nullptr), candidates_(nullptr) {}

Tree_builder::~Tree_builder() {
    delete[] candidates_;
    delete[] build_nodes_;
}

void Tree_builder::build(Tree& tree, Scene const& scene, Threads& threads) {
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

        split(tree, 0, num_infinite_lights, num_total_lights, scene, threads);

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

    float const infinite_weight = p0 / pt;

    tree.infinite_weight_ = infinite_weight;

    // This is because I'm afraid of the 1.f == random case
    tree.infinite_guard_ = 0 == num_finite_lights ? 1.1f : infinite_weight;
}

void Tree_builder::build(Primitive_tree& tree, Part const& part, Threads& threads) {
    uint32_t const num_finite_lights = part.num_triangles;

    tree.allocate_light_mapping(num_finite_lights);

    light_order_ = 0;

    uint32_t lm = 0;

    for (uint32_t l = 0, len = part.num_triangles; l < len; ++l) {
        tree.light_mapping_[lm++] = l;
    }

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

    split(tree, 0, 0, num_finite_lights, std::max(num_finite_lights / 32, 8u), part, threads);

    tree.allocate_nodes(current_node_);
    //    serialize(tree.nodes_, tree.node_middles_);
    serialize(tree, part);
}

uint32_t Tree_builder::split(Tree& tree, uint32_t node_id, uint32_t begin, uint32_t end,
                             Scene const& scene, Threads& threads) {
    uint32_t* const lights = tree.light_mapping_;

    Build_node& node = build_nodes_[node_id];

    uint32_t const len = end - begin;

    if (len <= 4) {
        AABB   bounds(Empty_AABB);
        float4 cone(1.f);
        float  total_power(0.f);

        for (uint32_t i = begin; i < end; ++i) {
            uint32_t const l = lights[i];

            bounds.merge_assign(scene.light_aabb(l));
            cone = cone::merge(cone, scene.light_cone(l));
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

    AABB   bounds(Empty_AABB);
    float4 cone(1.f);
    float  total_power(0.f);

    for (uint32_t i = begin; i < end; ++i) {
        uint32_t const l = lights[i];

        bounds.merge_assign(scene.light_aabb(l));
        cone = cone::merge(cone, scene.light_cone(l));
        total_power += scene.light_power(l);
    }

    float const  surface_area = bounds.surface_area();
    float const  cone_weight  = cone_importance(cone[3]);
    float3 const extent       = bounds.extent();
    float const  max_axis     = max_component(extent);

    float3 weights;
    uint3  split_nodes;

    static uint32_t constexpr Stride = 1;

    {
        uint32_t const sc = evaluate_splits(lights, begin, end, Stride, 0, candidates_, scene,
                                            threads);

        float const reg = max_axis / extent[0];

        weights[0]     = reg * candidates_[sc].weight / (surface_area * cone_weight);
        split_nodes[0] = candidates_[sc].split_node;
    }

    {
        uint32_t const sc = evaluate_splits(lights, begin, end, Stride, 1, candidates_, scene,
                                            threads);

        float const reg = max_axis / extent[1];

        weights[1]     = reg * candidates_[sc].weight / (surface_area * cone_weight);
        split_nodes[1] = candidates_[sc].split_node;
    }

    {
        uint32_t const sc = evaluate_splits(lights, begin, end, Stride, 2, candidates_, scene,
                                            threads);

        float const reg = max_axis / extent[2];

        weights[2]     = reg * candidates_[sc].weight / (surface_area * cone_weight);
        split_nodes[2] = candidates_[sc].split_node;
    }

    uint32_t const axis = index_min_component(weights);

    sort_lights(lights, begin, end, axis, scene);

    uint32_t const split_node = split_nodes[axis];

    uint32_t const c0_end = split(tree, child0, begin, split_node, scene, threads);
    uint32_t const c1_end = split(tree, child0 + 1, split_node, end, scene, threads);

    node.bounds            = bounds;
    node.cone              = cone;
    node.power             = total_power;
    node.variance          = variance(lights, begin, end, scene);
    node.middle            = c0_end;
    node.children_or_light = child0;
    node.num_lights        = len;

    return c1_end;
}

uint32_t Tree_builder::split(Primitive_tree& tree, uint32_t node_id, uint32_t begin, uint32_t end,
                             uint32_t max_primitives, Part const& part, Threads& threads) {
    uint32_t* const lights = tree.light_mapping_;

    Build_node& node = build_nodes_[node_id];

    uint32_t const len = end - begin;

    Simd_AABB tbounds(Empty_AABB);
    float4    cone(1.f);
    float     total_power(0.f);

    for (uint32_t i = begin; i < end; ++i) {
        uint32_t const l = lights[i];

        tbounds.merge_assign(part.light_aabb(l));
        cone = cone::merge(cone, part.light_cone(l));
        total_power += part.light_power(l);
    }

    AABB const bounds(tbounds);

    if (len <= max_primitives || cone[3] > 0.5f) {
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

        return begin + len;
    }

    uint32_t const child0 = current_node_;

    current_node_ += 2;

    float const  surface_area = bounds.surface_area();
    float const  cone_weight  = cone_importance(cone[3]);
    float3 const extent       = bounds.extent();
    float const  max_axis     = max_component(extent);

    float3 weights;
    uint3  split_nodes;

    uint32_t const stride = len < 64 ? 1 : len / 16;

    {
        uint32_t const sc = evaluate_splits(lights, begin, end, stride, 0, candidates_, part,
                                            threads);

        float const reg = max_axis / extent[0];

        weights[0]     = reg * candidates_[sc].weight / (surface_area * cone_weight);
        split_nodes[0] = candidates_[sc].split_node;
    }

    {
        uint32_t const sc = evaluate_splits(lights, begin, end, stride, 1, candidates_, part,
                                            threads);

        float const reg = max_axis / extent[1];

        weights[1]     = reg * candidates_[sc].weight / (surface_area * cone_weight);
        split_nodes[1] = candidates_[sc].split_node;
    }

    {
        uint32_t const sc = evaluate_splits(lights, begin, end, stride, 2, candidates_, part,
                                            threads);

        float const reg = max_axis / extent[2];

        weights[2]     = reg * candidates_[sc].weight / (surface_area * cone_weight);
        split_nodes[2] = candidates_[sc].split_node;
    }

    uint32_t const axis = index_min_component(weights);

    sort_lights(lights, begin, end, axis, part);

    uint32_t const split_node = split_nodes[axis];

    uint32_t const c0_end = split(tree, child0, begin, split_node, max_primitives, part, threads);
    uint32_t const c1_end = split(tree, child0 + 1, split_node, end, max_primitives, part, threads);

    node.bounds            = bounds;
    node.cone              = cone;
    node.power             = total_power;
    node.variance          = variance(lights, begin, end, part);
    node.middle            = c0_end;
    node.children_or_light = child0;
    node.num_lights        = len;

    return c1_end;
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
