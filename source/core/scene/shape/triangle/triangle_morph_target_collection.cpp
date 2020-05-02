#include "triangle_morph_target_collection.hpp"
#include "base/math/vector3.inl"
#include "base/thread/thread_pool.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "triangle_primitive.hpp"

namespace scene::shape::triangle {

const std::vector<Index_triangle>& Morph_target_collection::triangles() const {
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

void Morph_target_collection::morph(uint32_t a, uint32_t b, float weight, thread::Pool& threads,
                                    Vertex* vertices) {
    struct Args {
        Vertex const* va;
        Vertex const* vb;
        Vertex*       vertices;
        float         weight;
    };

    Args const args{morph_targets_[a].data(), morph_targets_[b].data(), vertices, weight};

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
        0, int32_t(morph_targets_[0].size()));
}

}  // namespace scene::shape::triangle
