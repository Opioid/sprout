#include "triangle_bvh_builder_sah.hpp"
#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "base/math/vector3.inl"
#include "base/thread/thread_pool.hpp"
#include "logging/logging.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/bvh/scene_bvh_split_candidate.inl"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "triangle_bvh_helper.hpp"
#include "triangle_bvh_tree.inl"

namespace scene::shape::triangle::bvh {

Builder_SAH::Build_node::Build_node() : start_index(0), end_index(0), children{nullptr, nullptr} {}

Builder_SAH::Build_node::~Build_node() {
    delete children[0];
    delete children[1];
}

Builder_SAH::Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold)
    : num_slices_(num_slices), sweep_threshold_(sweep_threshold) {}

void Builder_SAH::split(Build_node* node, References& references, AABB const& aabb,
                        uint32_t max_primitives, uint32_t depth, thread::Pool& thread_pool) {
    node->aabb = aabb;

    uint32_t const num_primitives = uint32_t(references.size());

    if (num_primitives <= max_primitives) {
        assign(node, references);
    } else {
        bool                  exhausted;
        Split_candidate const sp = splitting_plane(references, aabb, depth, exhausted, thread_pool);

        if (num_primitives <= 0xFF && (float(num_primitives) <= sp.cost() || exhausted)) {
            assign(node, references);
        } else {
            node->axis = sp.axis();

            References references0;
            References references1;
            sp.distribute(references, references0, references1);

            if (num_primitives <= 0xFF && (references0.empty() || references1.empty())) {
                // This can happen if we didn't find a good splitting plane.
                // It means every triangle was (partially) on the same side of the plane.
                assign(node, references);
            } else {
                if (exhausted) {
                    // TODO
                    // Implement a fallback solution that arbitrarily distributes the primitives
                    // to sub-nodes without needing a meaningful splitting plane.
                    logging::warning("Cannot split node further");
                    return;
                }

                ++depth;

                references = References();

                node->children[0] = new Build_node;
                split(node->children[0], references0, sp.aabb_0(), max_primitives, depth,
                      thread_pool);

                references0 = References();

                node->children[1] = new Build_node;
                split(node->children[1], references1, sp.aabb_1(), max_primitives, depth,
                      thread_pool);

                num_nodes_ += 2;
            }
        }
    }
}

Builder_SAH::Split_candidate Builder_SAH::splitting_plane(References const& references,
                                                          AABB const& aabb, uint32_t depth,
                                                          bool&         exhausted,
                                                          thread::Pool& thread_pool) {
    static uint8_t constexpr X = 0;
    static uint8_t constexpr Y = 1;
    static uint8_t constexpr Z = 2;

    split_candidates_.clear();

    uint32_t const num_triangles = uint32_t(references.size());

    float3 const halfsize = aabb.halfsize();
    float3 const position = aabb.position();

    split_candidates_.emplace_back(X, position, true);
    split_candidates_.emplace_back(Y, position, true);
    split_candidates_.emplace_back(Z, position, true);

    if (num_triangles <= sweep_threshold_) {
        for (auto const& r : references) {
            float3 const max(r.bounds[1].v);
            split_candidates_.emplace_back(X, max, false);
            split_candidates_.emplace_back(Y, max, false);
            split_candidates_.emplace_back(Z, max, false);
        }
    } else {
        float3 const& min = aabb.min();

        float3 const step = (2.f * halfsize) / float(num_slices_);
        for (uint32_t i = 1, len = num_slices_; i < len; ++i) {
            float const fi = float(i);

            float3 const slice_x(min[0] + fi * step[0], position[1], position[2]);
            split_candidates_.emplace_back(X, slice_x, false);

            float3 const slice_y(position[0], min[1] + fi * step[1], position[2]);
            split_candidates_.emplace_back(Y, slice_y, false);

            float3 const slice_z(position[0], position[1], min[2] + fi * step[2]);
            split_candidates_.emplace_back(Z, slice_z, false);

            if (depth < spatial_split_threshold_) {
                split_candidates_.emplace_back(X, slice_x, true);
                split_candidates_.emplace_back(Y, slice_y, true);
                split_candidates_.emplace_back(Z, slice_z, true);
            }
        }
    }

    float const aabb_surface_area = aabb.surface_area();

    // Arbitrary heuristic for starting the thread pool
    if (num_triangles < 1024) {
        for (auto& sc : split_candidates_) {
            sc.evaluate(references, aabb_surface_area);
        }
    } else {
        thread_pool.run_range(
            [this, &references, aabb_surface_area](uint32_t /*id*/, int32_t sc_begin,
                                                   int32_t sc_end) {
                for (int32_t i = sc_begin; i < sc_end; ++i) {
                    split_candidates_[uint32_t(i)].evaluate(references, aabb_surface_area);
                }
            },
            0, static_cast<int32_t>(split_candidates_.size()));
    }

    size_t sc       = 0;
    float  min_cost = split_candidates_[0].cost();

    for (size_t i = 1, len = split_candidates_.size(); i < len; ++i) {
        float const cost = split_candidates_[i].cost();

        if (cost < min_cost) {
            sc       = i;
            min_cost = cost;
        }
    }

    auto const& sp = split_candidates_[sc];

    exhausted = (sp.aabb_0() == aabb && num_triangles == sp.num_side_0()) ||
                (sp.aabb_1() == aabb && num_triangles == sp.num_side_1());

    return sp;
}

scene::bvh::Node& Builder_SAH::new_node() {
    return nodes_[current_node_++];
}

uint32_t Builder_SAH::current_node_index() const {
    return current_node_;
}

void Builder_SAH::assign(Build_node* node, References const& references) {
    size_t const num_references = references.size();
    node->primitives.resize(num_references);
    for (size_t i = 0; i < num_references; ++i) {
        node->primitives[i] = references[i].primitive();
    }

    node->start_index = num_references_;
    num_references_ += uint32_t(num_references);
    node->end_index = num_references_;
}

}  // namespace scene::shape::triangle::bvh
