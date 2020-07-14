#include "scene_bvh_builder_base.hpp"
#include "base/math/aabb.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "logging/logging.hpp"
#include "scene_bvh_node.inl"
#include "scene_bvh_split_candidate.inl"

namespace scene::bvh {

Builder_base::Build_node::Build_node() = default;

Builder_base::Build_node::~Build_node() {
    memory::free_aligned(primitives);

    delete children[0];
    delete children[1];
}

void Builder_base::Build_node::allocate(uint8_t num_primitives) {
    num_indices = num_primitives;

    primitives = memory::allocate_aligned<uint32_t>(num_primitives);
}

Builder_base::Builder_base(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives)
    : num_slices_(num_slices), sweep_threshold_(sweep_threshold), max_primitives_(max_primitives) {}

void Builder_base::split(Build_node* node, References& references, AABB const& aabb, uint32_t depth,
                         thread::Pool& threads) {
    node->aabb = aabb;

    uint32_t const num_primitives = uint32_t(references.size());

    if (num_primitives <= max_primitives_) {
        assign(node, references);
    } else {
        bool                  exhausted;
        Split_candidate const sp = splitting_plane(references, aabb, depth, exhausted, threads);

        if (num_primitives <= 0xFF && (float(num_primitives) <= sp.cost() || exhausted)) {
            assign(node, references);
        } else {
            if (exhausted) {
                // TODO
                // Implement a fallback solution that arbitrarily distributes the primitives
                // to sub-nodes without needing a meaningful splitting plane.
                logging::warning("Cannot split node further");
                return;
            }

            node->axis = sp.axis();

            References references0;
            References references1;
            sp.distribute(references, references0, references1);

            if (num_primitives <= 0xFF && (references0.empty() || references1.empty())) {
                // This can happen if we didn't find a good splitting plane.
                // It means every triangle was (partially) on the same side of the plane.
                assign(node, references);
            } else {
                ++depth;

                references = References();

                node->children[0] = new Build_node;
                split(node->children[0], references0, sp.aabb_0(), depth, threads);

                references0 = References();

                node->children[1] = new Build_node;
                split(node->children[1], references1, sp.aabb_1(), depth, threads);

                num_nodes_ += 2;
            }
        }
    }
}

Split_candidate Builder_base::splitting_plane(References const& references, AABB const& aabb,
                                              uint32_t depth, bool& exhausted,
                                              thread::Pool& threads) {
    static uint8_t constexpr X = 0;
    static uint8_t constexpr Y = 1;
    static uint8_t constexpr Z = 2;

    split_candidates_.clear();

    uint32_t const num_references = uint32_t(references.size());

    float3 const position = aabb.position();

    split_candidates_.emplace_back(X, position, true);
    split_candidates_.emplace_back(Y, position, true);
    split_candidates_.emplace_back(Z, position, true);

    if (num_references <= sweep_threshold_) {
        for (auto const& r : references) {
            float3 const max(r.bounds[1].v);
            split_candidates_.emplace_back(X, max, false);
            split_candidates_.emplace_back(Y, max, false);
            split_candidates_.emplace_back(Z, max, false);
        }
    } else {
        float3 const extent = 2.f * aabb.halfsize();

        float3 const min = aabb.min();

        uint32_t const la = index_max_component(extent);

        float const step = (extent[la]) / float(num_slices_);

        for (uint32_t a = 0; a < 3; ++a) {
            float const extent_a = extent[a];

            uint32_t const num_steps = uint32_t(std::ceil(extent_a / step));

            float const step_a = extent_a / float(num_steps);

            for (uint32_t i = 1; i < num_steps; ++i) {
                float const fi = float(i);

                float3 slice = position;
                slice[a]     = min[a] + fi * step_a;
                split_candidates_.emplace_back(a, slice, false);

                if (depth < spatial_split_threshold_) {
                    split_candidates_.emplace_back(a, slice, true);
                }
            }
        }
    }

    float const aabb_surface_area = aabb.surface_area();

    // Arbitrary heuristic for starting the thread pool
    if (num_references < 1024) {
        for (auto& sc : split_candidates_) {
            sc.evaluate(references, aabb_surface_area);
        }
    } else {
        threads.run_range(
            [&scs = split_candidates_, &references, aabb_surface_area](
                uint32_t /*id*/, int32_t sc_begin, int32_t sc_end) noexcept {
                for (int32_t i = sc_begin; i < sc_end; ++i) {
                    scs[uint32_t(i)].evaluate(references, aabb_surface_area);
                }
            },
            0, int32_t(split_candidates_.size()));
    }

    size_t sc = 0;

    float min_cost = split_candidates_[0].cost();

    for (size_t i = 1, len = split_candidates_.size(); i < len; ++i) {
        if (float const cost = split_candidates_[i].cost(); cost < min_cost) {
            sc = i;

            min_cost = cost;
        }
    }

    auto const& sp = split_candidates_[sc];

    exhausted = (sp.aabb_0() == aabb && num_references == sp.num_side_0()) ||
                (sp.aabb_1() == aabb && num_references == sp.num_side_1());

    return sp;
}

void Builder_base::assign(Build_node* node, References const& references) {
    uint8_t const num_references = uint8_t(references.size());

    node->allocate(num_references);

    for (uint8_t i = 0; i < num_references; ++i) {
        node->primitives[i] = references[i].primitive();
    }

    node->start_index = num_references_;
    node->num_indices = num_references;

    num_references_ += uint32_t(num_references);
}

bvh::Node& Builder_base::new_node() {
    return nodes_[current_node_++];
}

uint32_t Builder_base::current_node_index() const {
    return current_node_;
}

}  // namespace scene::bvh
