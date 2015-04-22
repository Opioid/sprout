#include "triangle_bvh_xtree.hpp"
#include "scene/shape/triangle/triangle_primitive.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "base/math/vector.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

XNode::Children XNode::children(uint8_t sign, uint32_t id) const {
	if (0 == sign) {
		return Children{id + 1, end_index};
	} else {
		return Children{end_index, id + 1};
	}
}

bool XNode::has_children() const {
	return has_children_flag == (start_index & has_children_flag);
}

void XNode::set_has_children(bool children) {
	if (children) {
		start_index |= has_children_flag;
	} else {
		start_index &= ~has_children_flag;
	}
}

void XNode::set_right_child(uint32_t offset) {
	end_index = offset;
}

const math::AABB& Tree::aabb() const {
	return nodes_[0].aabb;
}

bool XTree::intersect(math::Oray& ray, const math::float2& /*bounds*/, Intersection& intersection) const {
	return intersect_node(0, ray, intersection);
}

bool XTree::intersect_p(const math::Oray& ray, const math::float2& /*bounds*/) const {
	return intersect_node_p(0, ray);
}

void XTree::interpolate_triangle(uint32_t index, float u, float v, math::float3& n, math::float3& t, math::float2& uv) const {
	triangles_[index].interpolate(u, v, n, t, uv);
}

uint32_t XTree::triangle_material_index(uint32_t index) const {
	return triangles_[index].material_index;
}

bool XTree::intersect_node(uint32_t n, math::Oray& ray, Intersection& intersection) const {
	auto& node = nodes_[n];

	if (!node.aabb.intersect_p(ray)) {
		return false;
	}

	bool hit = false;

	if (node.has_children()) {
		auto children = node.children(ray.sign[node.axis], n);

		if (intersect_node(children.a, ray, intersection)) {
			hit = true;
		}

		if (intersect_node(children.b, ray, intersection)) {
			hit = true;
		}
	} else {
		Coordinates c;
		Intersection ti;
		ti.c.t = ray.max_t;

		for (uint32_t i = node.start_index; i < node.end_index; ++i) {
			if (triangles_[i].intersect(ray, c)) {
				if (c.t < ti.c.t) {
					ti.c = c;
					ti.index = i;
					hit = true;
				}
			}

		}

		if (hit) {
			intersection = ti;
			ray.max_t = ti.c.t;
		}
	}

	return hit;
}

bool XTree::intersect_node_p(uint32_t n, const math::Oray& ray) const {
	auto& node = nodes_[n];

	if (!node.aabb.intersect_p(ray)) {
		return false;
	}

	if (node.has_children()) {
		auto children = node.children(ray.sign[node.axis], n);

		if (intersect_node_p(children.a, ray)) {
			return true;
		}

		return intersect_node_p(children.b, ray);
	}

	for (uint32_t i = node.start_index; i < node.end_index; ++i) {
		if (triangles_[i].intersect_p(ray)) {
			return true;
		}
	}

	return false;
}

std::vector<XNode>& Tree::allocate_nodes(uint32_t num_nodes) {
	nodes_.resize(num_nodes);
	return nodes_;
}

}}}}




