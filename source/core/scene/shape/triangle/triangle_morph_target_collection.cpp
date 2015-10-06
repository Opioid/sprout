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

void Morph_target_collection::add_swap_vertices(std::vector<Vertex>& vertices) {
	morph_targets_.push_back(std::vector<Vertex>());
	morph_targets_.back().swap(vertices);
}

}}}
