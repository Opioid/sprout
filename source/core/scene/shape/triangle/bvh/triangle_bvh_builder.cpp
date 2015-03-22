#include "triangle_bvh_builder.hpp"
#include "triangle_bvh_tree.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/vector.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

void Builder::build(Tree& tree, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices) {
	tree.triangles_.resize(triangles.size());

	for (size_t i = 0, len = triangles.size(); i < len; ++i) {
		auto& t = triangles[i];
		tree.triangles_[i] = Triangle{vertices[t.a], vertices[t.b], vertices[t.c], t.material_index};
	}

	std::vector<uint32_t> primitive_indices(triangles.size());
	for (size_t i = 0, len = primitive_indices.size(); i < len; ++i) {
		primitive_indices[i] = static_cast<uint32_t>(i);
	}

	tree.aabb_ = submesh_aabb(primitive_indices, triangles, vertices);
}

math::float3 triangle_min(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& x) {
	return math::min(a, math::min(b, math::min(c, x)));
}

math::float3 triangle_max(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& x) {
	return math::max(a, math::max(b, math::max(c, x)));
}

math::AABB Builder::submesh_aabb(const std::vector<uint32_t>& primitive_indices, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices) {
	float max_float = std::numeric_limits<float>::max();
	math::float3 min(max_float, max_float, max_float);
	math::float3 max(-max_float, -max_float, -max_float);

	for (size_t i = 0, len = primitive_indices.size(); i < len; ++i) {
		uint32_t pi = primitive_indices[i];
		min = triangle_min(vertices[triangles[pi].a].p, vertices[triangles[pi].b].p, vertices[triangles[pi].c].p, min);
		max = triangle_max(vertices[triangles[pi].a].p, vertices[triangles[pi].b].p, vertices[triangles[pi].c].p, max);
	}

	const static float epsilon = 0.000000001f;

	max.x += epsilon;
	max.y += epsilon;
	max.z += epsilon;

	return math::AABB(min, max);
}

}}}}



