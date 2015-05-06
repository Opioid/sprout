#include "triangle_bvh_tree.hpp"
#include "scene/shape/triangle/triangle_primitive.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "base/math/sampling.hpp"
#include "base/math/vector.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

Node::Children Node::children(uint8_t sign, uint32_t id) const {
	if (0 == sign) {
		return Children{id + 1, end_index};
	} else {
		return Children{end_index, id + 1};
	}
}

bool Node::has_children() const {
	return has_children_flag == (start_index & has_children_flag);
}

void Node::set_has_children(bool children) {
	if (children) {
		start_index |= has_children_flag;
	} else {
		start_index &= ~has_children_flag;
	}
}

void Node::set_right_child(uint32_t offset) {
	end_index = offset;
}

const math::AABB& Tree::aabb() const {
	return nodes_[0].aabb;
}

uint32_t Tree::num_parts() const {
	return num_parts_;
}

bool Tree::intersect(math::Oray& ray, const math::float2& /*bounds*/, Node_stack& /*node_stack*/, Intersection& intersection) const {
	return intersect_node(0, ray, intersection);
}

bool Tree::intersect_p(const math::Oray& ray, const math::float2& /*bounds*/, Node_stack& /*node_stack*/) const {
	return intersect_node_p(0, ray);
}

void Tree::interpolate_triangle_data(uint32_t index, math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const {
	triangles_[index].interpolate_data(uv, n, t, tc);
}

uint32_t Tree::triangle_material_index(uint32_t index) const {
	return triangles_[index].material_index;
}

void Tree::importance_sample(float r, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const {
	float num = static_cast<float>(triangles_.size());
	size_t index = static_cast<size_t>(num * r - 0.001f);
	triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, n, tc);
}

bool Tree::intersect_node(uint32_t n, math::Oray& ray, Intersection& intersection) const {
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

bool Tree::intersect_node_p(uint32_t n, const math::Oray& ray) const {
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

std::vector<Node>& Tree::allocate_nodes(uint32_t num_nodes) {
	nodes_.resize(num_nodes);
	return nodes_;
}

}}}}




