#include "triangle_bvh_xtree.hpp"
#include "scene/shape/triangle/triangle_primitive.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "base/math/sampling.hpp"
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

const math::AABB& XTree::aabb() const {
	return nodes_[0].aabb;
}

bool XTree::intersect(math::Oray& ray, const math::float2& /*bounds*/, Node_stack& node_stack, Intersection& intersection) const {
	node_stack.clear();
	node_stack.push_back(0);

	bool hit = false;

	while (!node_stack.empty()) {
		uint32_t n = node_stack.back();
		node_stack.pop_back();

		auto& node = nodes_[n];

		if (node.aabb.intersect_p(ray)) {
			if (node.has_children()) {
				auto children = node.children(ray.sign[node.axis], n);
				node_stack.push_back(children.b);
				node_stack.push_back(children.a);
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
		}
	}

	return hit;
}

bool XTree::intersect_p(const math::Oray& ray, const math::float2& /*bounds*/, Node_stack& node_stack) const {
	node_stack.clear();
	node_stack.push_back(0);

	while (!node_stack.empty()) {
		uint32_t n = node_stack.back();
		node_stack.pop_back();
		auto& node = nodes_[n];

		if (node.aabb.intersect_p(ray)) {
			if (node.has_children()) {
				auto children = node.children(ray.sign[node.axis], n);
				node_stack.push_back(children.b);
				node_stack.push_back(children.a);
			} else {
				for (uint32_t i = node.start_index; i < node.end_index; ++i) {
					if (triangles_[i].intersect_p(ray)) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

void XTree::interpolate_triangle_data(uint32_t index, math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const {
	triangles_[index].interpolate_data(uv, n, t, tc);
}

uint32_t XTree::triangle_material_index(uint32_t index) const {
	return triangles_[index].material_index;
}

void XTree::importance_sample(float r, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const {
	float num = static_cast<float>(triangles_.size());
	size_t index = static_cast<size_t>(num * r - 0.001f);
	triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, n, tc);
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

std::vector<XNode>& XTree::allocate_nodes(uint32_t num_nodes) {
	nodes_.resize(num_nodes);
	return nodes_;
}

}}}}




