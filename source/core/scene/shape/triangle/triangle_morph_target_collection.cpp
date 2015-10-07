#include "triangle_morph_target_collection.hpp"
#include "triangle_primitive.hpp"
#include "scene/shape/geometry/vertex.hpp"

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

void Morph_target_collection::morph(uint32_t a, uint32_t b, float delta, std::vector<Vertex>& vertices) {
	auto& va = morph_targets_[a];
	auto& vb = morph_targets_[b];

	for (size_t i = 0, len = va.size(); i < len; ++i) {
		vertices[i].p = math::lerp(va[i].p, vb[i].p, delta);
		vertices[i].n = math::normalized(math::lerp(va[i].n, vb[i].n, delta));
		vertices[i].t = math::normalized(math::lerp(va[i].t, vb[i].t, delta));
		vertices[i].uv = va[i].uv;
	}
}

}}}
