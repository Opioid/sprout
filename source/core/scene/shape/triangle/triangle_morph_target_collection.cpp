#include "triangle_morph_target_collection.hpp"
#include "base/math/vector3.inl"
#include "base/thread/thread_pool.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "triangle_primitive.hpp"

namespace scene::shape::triangle {

std::vector<Index_triangle>& Morph_target_collection::triangles() {
    return triangles_;
}

const std::vector<Index_triangle>& Morph_target_collection::triangles() const {
    return triangles_;
}

const std::vector<Vertex>& Morph_target_collection::vertices(uint32_t target) const {
    return morph_targets_[target];
}

void Morph_target_collection::add_swap_vertices(std::vector<Vertex>& vertices) {
    morph_targets_.push_back(std::vector<Vertex>());
    morph_targets_.back().swap(vertices);
}

void Morph_target_collection::morph(uint32_t a, uint32_t b, float weight, thread::Pool& pool,
                                    std::vector<Vertex>& vertices) {
    auto const& va = morph_targets_[a];
    auto const& vb = morph_targets_[b];

    pool.run_range(
        [&va, &vb, weight, &vertices](uint32_t /*id*/, int32_t begin, int32_t end) {
            for (int32_t i = begin; i < end; ++i) {
                vertices[i].p  = math::lerp(va[i].p, vb[i].p, weight);
                vertices[i].n  = normalize(math::lerp(va[i].n, vb[i].n, weight));
                vertices[i].t  = normalize(math::lerp(va[i].t, vb[i].t, weight));
                vertices[i].uv = math::lerp(va[i].uv, vb[i].uv, weight);
            }
        },
        0, static_cast<int32_t>(va.size()));
}

}  // namespace scene::shape::triangle
