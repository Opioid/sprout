#include "triangle_morph_target_collection.hpp"
#include "base/math/vector3.inl"
#include "base/thread/thread_pool.hpp"
#include "scene/animation/animation.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "triangle_primitive.hpp"

#include <algorithm>

namespace scene::shape::triangle {

Morph_target_collection::Morph_target_collection() = default;

Morph_target_collection::Morph_target_collection(Morph_target_collection&& other)
    : triangles_(std::move(other.triangles_)), morph_targets_(std::move(other.morph_targets_)) {}

std::vector<Index_triangle> const& Morph_target_collection::triangles() const {
    return triangles_;
}

std::vector<Index_triangle>& Morph_target_collection::triangles() {
    return triangles_;
}

void Morph_target_collection::add_swap_vertices(std::vector<Vertex>& vertices) {
    morph_targets_.emplace_back(std::vector<Vertex>());
    morph_targets_.back().swap(vertices);
}

uint32_t Morph_target_collection::num_vertices() const {
    return uint32_t(morph_targets_[0].size());
}

void Morph_target_collection::morph(Keyframe const* frames, uint32_t num_frames, Threads& threads,
                                    Vertex* vertices) {
    for (uint32_t i = 0; i < num_frames; ++i) {
        morph(frames[i], threads, vertices + i * num_vertices());
    }
}

void Morph_target_collection::morph(Keyframe const& f, Threads& threads, Vertex* vertices) {
    auto const& m = f.m;

    if (0.f == m.weight) {
        Vertex const* source = morph_targets_[m.targets[0]].data();

        std::copy(source, source + num_vertices(), vertices);

        return;
    }

    struct Args {
        Vertex const* va;
        Vertex const* vb;

        Vertex* vertices;
        float   weight;
    };

    Args const args{morph_targets_[m.targets[0]].data(), morph_targets_[m.targets[1]].data(),
                    vertices, m.weight};

    threads.run_range(
        [&args](uint32_t /*id*/, int32_t begin, int32_t end) {
            for (int32_t i = begin; i < end; ++i) {
                args.vertices[i].p  = lerp(args.va[i].p, args.vb[i].p, args.weight);
                args.vertices[i].n  = normalize(lerp(args.va[i].n, args.vb[i].n, args.weight));
                args.vertices[i].t  = normalize(lerp(args.va[i].t, args.vb[i].t, args.weight));
                args.vertices[i].uv = lerp(args.va[i].uv, args.vb[i].uv, args.weight);

                args.vertices[i].bitangent_sign = args.va[i].bitangent_sign;
            }
        },
        0, int32_t(num_vertices()));
}

}  // namespace scene::shape::triangle
