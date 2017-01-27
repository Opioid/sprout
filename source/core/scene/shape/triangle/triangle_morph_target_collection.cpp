#include "triangle_morph_target_collection.hpp"
#include "triangle_primitive.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "base/math/vector.inl"
#include "base/thread/thread_pool.hpp"

namespace scene { namespace shape { namespace triangle {

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
	const auto& va = morph_targets_[a];
	const auto& vb = morph_targets_[b];

	pool.run_range([&va, &vb, weight, &vertices](uint32_t /*id*/, int32_t begin, int32_t end) {
		for (uint32_t i = begin; i < end; ++i) {
			vertices[i].p = math::lerp(va[i].p, vb[i].p, weight);
			vertices[i].n = math::normalized(math::lerp(va[i].n, vb[i].n, weight));
			vertices[i].t = math::normalized(math::lerp(va[i].t, vb[i].t, weight));
			vertices[i].uv = math::lerp(va[i].uv, vb[i].uv, weight);
		}
	}, 0, static_cast<int32_t>(va.size()));
}

}}}
