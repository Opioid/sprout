#include "scene_bvh_builder.hpp"
#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "base/memory/array.inl"
#include "scene/scene.inl"
#include "scene_bvh_node.inl"
#include "scene_bvh_split_candidate.inl"
#include "scene_bvh_tree.inl"
#include "base/thread/thread_pool.hpp"
#include "logging/logging.hpp"

namespace scene::bvh {

Builder::Builder() noexcept : root_(new Build_node), num_slices_(16), sweep_threshold_(64) {
    root_->clear();
}

Builder::~Builder() noexcept {
    delete root_;
}

void Builder::build(Tree& tree, std::vector<uint32_t>& indices,
                    std::vector<AABB> const& aabbs, thread::Pool& pool) noexcept {
    if (indices.empty()) {
        nodes_ = tree.allocate_nodes(0);
    } else {
        float const log2_num_triangles = std::log2(float(indices.size()));

        spatial_split_threshold_ = uint32_t(log2_num_triangles / 2.f + 0.5f);

        References references(indices.size());

           memory::Array<Simd_AABB> taabbs(pool.num_threads());

           pool.run_range(
               [&indices, &aabbs, &references, &taabbs](uint32_t id, int32_t begin, int32_t end) {
                   AABB aabb(AABB::empty());

                   for (int32_t i = begin; i < end; ++i) {
                       uint32_t const prop = indices[uint32_t(i)];

                       AABB const& b = aabbs[prop];

                       float3 const& min = b.min();
                       float3 const& max = b.max();

                       references[uint32_t(i)].set(min, max, prop);

                       aabb.merge_assign(b);
                   }

                   taabbs[id] = aabb;
               },
               0, int32_t(indices.size()));

           Simd_AABB aabb(AABB::empty());
           for (auto& b : taabbs) {
               aabb.merge_assign(b);
           }






        num_nodes_ = 1;
        num_references_ = 0;

        split(root_, references, AABB(aabb.min, aabb.max), 4, 0, pool);

        tree.alllocate_indices(num_references_);

        nodes_ = tree.allocate_nodes(num_nodes_);

        current_node_ = 0;

        uint32_t current_prop = 0;
        serialize(root_, tree, current_prop);
    }

    tree.aabb_ = root_->aabb;

    root_->clear();
}

Builder::Build_node::~Build_node() noexcept {
    delete children[0];
    delete children[1];
}

void Builder::Build_node::clear() noexcept {
    delete children[0];
    children[0] = nullptr;

    delete children[1];
    children[1] = nullptr;

    props_end = 0;
    offset    = 0;

    start_index = 0;
    end_index = 0;

    // This size will be used even if there are only infinite props in the scene.
    // It is an arbitrary size that will be used to calculate the power of some lights.
    aabb = AABB(float3(-1.f), float3(1.f));
}
void Builder::split(Build_node* node, References& references, AABB const& aabb,
                        uint32_t max_primitives, uint32_t depth, thread::Pool& pool) {
    node->aabb = aabb;

    uint32_t const num_primitives = uint32_t(references.size());

    if (num_primitives <= max_primitives) {
        assign(node, references);
    } else {
        bool                  exhausted;
        Split_candidate const sp = splitting_plane(references, aabb, depth, exhausted, pool);

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
                      pool);

                references0 = References();

                node->children[1] = new Build_node;
                split(node->children[1], references1, sp.aabb_1(), max_primitives, depth,
                      pool);

                num_nodes_ += 2;
            }
        }
    }
}


Split_candidate Builder::splitting_plane(References const& references,
                                                          AABB const& aabb, uint32_t depth,
                                                          bool&         exhausted,
                                                          thread::Pool& pool) {
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
        pool.run_range(
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

void Builder::serialize(Build_node* node, Tree& tree, uint32_t& current_prop) noexcept {
    auto& n = new_node();
    n.set_aabb(node->aabb.min().v, node->aabb.max().v);

    if (node->children[0]) {
        serialize(node->children[0], tree, current_prop);

        n.set_split_node(current_node_index(), node->axis);

        serialize(node->children[1], tree, current_prop);
    } else {
        uint8_t const num_primitives = uint8_t(node->end_index - node->start_index);
        n.set_leaf_node(node->start_index, num_primitives);

        uint32_t i = current_prop;
        for (auto const p : node->primitives) {
            tree.indices_[i] = p;
            ++i;
        }

        current_prop = i;
    }
}

bvh::Node& Builder::new_node() noexcept {
    return nodes_[current_node_++];
}

uint32_t Builder::current_node_index() const noexcept {
    return current_node_;
}

void Builder::assign(Build_node* node, References const& references) {
    size_t const num_references = references.size();
    node->primitives.resize(num_references);
    for (size_t i = 0; i < num_references; ++i) {
        node->primitives[i] = references[i].primitive();
    }

    node->start_index = num_references_;
    num_references_ += uint32_t(num_references);
    node->end_index = num_references_;
}


}  // namespace scene::bvh
