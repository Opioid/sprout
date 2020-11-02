#include "scene_bvh_builder_base.hpp"
#include "base/math/aabb.inl"
#include "base/memory/align.hpp"
#include "base/thread/thread_pool.hpp"
#include "logging/logging.hpp"
#include "scene_bvh_node.inl"
#include "scene_bvh_split_candidate.inl"

namespace scene::bvh {

static uint32_t constexpr Parallelize_threshold = 1024;

Kernel::Kernel(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives,
               uint32_t spatial_split_threshold)
    : num_slices_(num_slices),
      sweep_threshold_(sweep_threshold),
      max_primitives_(max_primitives),
      spatial_split_threshold_(spatial_split_threshold) {
    split_candidates_.reserve(std::max(3 * sweep_threshold, 3 * num_slices));
}

Kernel::~Kernel() = default;

void Kernel::split(uint32_t node_id, References& references, AABB const& aabb, uint32_t depth,
                   Threads& threads, Tasks& tasks) {
    Node& node = build_nodes_[node_id];

    node.set_aabb(aabb);

    uint32_t const num_primitives = uint32_t(references.size());

    if (num_primitives <= max_primitives_) {
        assign(node, references);
    } else {
        if (!threads.is_running_parallel() && tasks.capacity() > 0 &&
            (num_primitives < Parallelize_threshold || depth == parallel_build_depth_)) {
            tasks.emplace_back(Task(new Kernel(num_slices_, sweep_threshold_, max_primitives_,
                                               spatial_split_threshold_),
                                    node_id, depth, aabb, std::move(references)));

            return;
        }

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

            References references0;
            References references1;
            sp.distribute(references, references0, references1);

            if (num_primitives <= 0xFF && (references0.empty() || references1.empty())) {
                // This can happen if we didn't find a good splitting plane.
                // It means every triangle was (partially) on the same side of the plane.
                assign(node, references);
            } else {
                ++depth;

                references.release();

                uint32_t const child0 = uint32_t(build_nodes_.size());

                build_nodes_[node_id].set_split_node(child0, sp.axis());

                build_nodes_.emplace_back();
                build_nodes_.emplace_back();

                split(child0, references0, sp.aabb_0(), depth, threads, tasks);

                references0.release();

                split(child0 + 1, references1, sp.aabb_1(), depth, threads, tasks);
            }
        }
    }
}

Split_candidate Kernel::splitting_plane(References const& references, AABB const& aabb,
                                        uint32_t depth, bool& exhausted, Threads& threads) {
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

        for (uint8_t a = 0; a < 3; ++a) {
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
    if (threads.is_running_parallel() || num_references < Parallelize_threshold) {
        for (auto& sc : split_candidates_) {
            sc.evaluate(references, aabb_surface_area);
        }
    } else {
        threads.run_range(
            [& scs = split_candidates_, &references, aabb_surface_area](
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

void Kernel::assign(Node& node, References const& references) {
    uint8_t const num_references = uint8_t(references.size());

    node.set_leaf_node(uint32_t(reference_ids_.size()), num_references);

    for (uint8_t i = 0; i < num_references; ++i) {
        reference_ids_.push_back(references[i].primitive());
    }
}

void Kernel::reserve(uint32_t num_primitives) {
    build_nodes_.reserve(std::max((3 * num_primitives) / max_primitives_, 1u));
    build_nodes_.clear();
    build_nodes_.emplace_back();

    reference_ids_.reserve((num_primitives * 12) / 10);
}

void Builder_base::work_on_tasks(Threads& threads, Tasks& tasks) {
    if (tasks.empty()) {
        return;
    }

    current_task_ = 0;

    threads.run_parallel(
        [this, &threads, &tasks](uint32_t /*id*/) noexcept {
            uint32_t const num_tasks = uint32_t(tasks.size());

            for (;;) {
                uint32_t const current = current_task_.fetch_add(1, std::memory_order_relaxed);

                if (current >= num_tasks) {
                    return;
                }

                Task& t = tasks[current];

                t.kernel->reserve(t.references.size());
                t.kernel->split(0, t.references, t.aabb, t.depth, threads, tasks);
                t.references.release();
            }
        },
        uint32_t(tasks.size()));

    for (auto const& task : tasks) {
        std::vector<Node> const& children = task.kernel->build_nodes_;

        Node& parent = build_nodes_[task.root];

        parent = children[0];

        if (1 == children.size()) {
            continue;
        }

        uint32_t const node_offset = uint32_t(build_nodes_.size() - 1);

        uint32_t const reference_offset = uint32_t(reference_ids_.size());

        reference_ids_.insert(reference_ids_.end(), task.kernel->reference_ids_.begin(),
                              task.kernel->reference_ids_.end());

        parent.offset(node_offset);

        for (size_t c = 1, len = children.size(); c < len; ++c) {
            Node const& sn = children[c];

            build_nodes_.emplace_back(sn, 0 == sn.num_indices() ? node_offset : reference_offset);
        }
    }
}

Kernel::Task::Task() = default;

Kernel::Task::Task(Kernel* k, uint32_t rt, uint32_t d, AABB const& box, References&& refs)
    : kernel(k), root(rt), depth(d), aabb(box), references(std::move(refs)) {}

Kernel::Task::Task(Task&& other)
    : kernel(other.kernel),
      root(other.root),
      depth(other.depth),
      aabb(other.aabb),
      references(std::move(other.references)) {
    other.kernel = nullptr;
}

Kernel::Task::~Task() {
    delete kernel;
}

Builder_base::Builder_base(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives)
    : Kernel(num_slices, sweep_threshold, max_primitives) {}

Builder_base::~Builder_base() {}

void Builder_base::split(References& references, AABB const& aabb, Threads& threads) {
    float const log2_num_references = std::log2(float(references.size()));

    spatial_split_threshold_ = uint32_t(std::lrint(log2_num_references / 2.f));

    parallel_build_depth_ = std::min(spatial_split_threshold_, 6u);

    uint32_t const num_tasks = std::min(math::exp2(parallel_build_depth_),
                                        references.size() / Parallelize_threshold);

    Tasks tasks;

    if (num_tasks > 2) {
        tasks.reserve(num_tasks);
    }

    Kernel::split(0, references, aabb, 0, threads, tasks);

    work_on_tasks(threads, tasks);
}

void Builder_base::reserve(uint32_t num_primitives) {
    Kernel::reserve(num_primitives);

    current_node_ = 0;
}

void Builder_base::new_node() {
    current_node_++;
}

uint32_t Builder_base::current_node_index() const {
    return current_node_;
}

}  // namespace scene::bvh
