#include "triangle_bvh_tree.hpp"
#include "scene/shape/triangle/triangle_primitive.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "base/math/vector.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

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

bool Tree::intersect(math::Oray& ray, const math::float2& bounds, Intersection& intersection) const {
	bool hit = false;

	Coordinates c;

	Intersection ti;
	ti.c.t = ray.max_t;

	for (size_t i = 0; i < triangles_.size(); ++i) {
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

	return hit;
}

bool Tree::intersect_p(const math::Oray& ray, const math::float2& bounds) const {
	for (size_t i = 0; i < triangles_.size(); ++i) {
		if (triangles_[i].intersect_p(ray)) {
			return true;
		}
	}

	return false;
}

bool Tree::intersect_node(uint32_t n, math::Oray& ray, Intersection& intersection) const {

	return false;
}

std::vector<Node>& Tree::allocate_nodes(uint32_t num_nodes) {
	nodes_.resize(num_nodes);
	return nodes_;
}

}}}}




