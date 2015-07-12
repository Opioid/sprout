#include "triangle_bvh_tree.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/shape/triangle/triangle_primitive.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "base/math/sampling.inl"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

uint32_t Node::axis() const {
	return start_index & ~has_children_flag;
}

void Node::set_axis(uint32_t axis) {
	start_index |= axis;
}

Node::Children Node::children(int sign, uint32_t id) const {
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

const math::aabb& Tree::aabb() const {
	return nodes_[0].aabb;
}

uint32_t Tree::num_parts() const {
	return num_parts_;
}

uint32_t Tree::num_triangles() const {
	return static_cast<uint32_t>(triangles_.size());

//	return static_cast<uint32_t>(itriangles_.size());

//	return static_cast<uint32_t>(ptriangles_.size());
}

const std::vector<Triangle>& Tree::triangles() const {
	return triangles_;
}

bool Tree::intersect(math::Oray& ray, const math::float2& /*bounds*/, Node_stack& node_stack, Intersection& intersection) const {
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	math::float2 uv;
	uint32_t index = 0xFFFFFFFF;

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.aabb.intersect_p(ray)) {
			if (node.has_children()) {
				auto children = node.children(ray.sign[node.axis()], n);
				node_stack.push(children.b);
				n = children.a;
			} else {
				for (uint32_t i = node.start_index; i < node.end_index; ++i) {
					if (triangles_[i].intersect(ray, uv)) {
						index = i;
					}

//					if (triangle::intersect(vertices_[itriangles_[i].a].p,
//											vertices_[itriangles_[i].b].p,
//											vertices_[itriangles_[i].c].p,
//											ray, uv)) {
//						index = i;
//					}

//					if (ptriangles_[i].intersect(ray, uv)) {
//						index = i;
//					}
				}

				n = node_stack.pop();
			}
		} else {
			n = node_stack.pop();
		}
	}

	intersection.uv = uv;
	intersection.index = index;

	return index != 0xFFFFFFFF;
}

bool Tree::intersect_p(const math::Oray& ray, const math::float2& /*bounds*/, Node_stack& node_stack) const {
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.aabb.intersect_p(ray)) {
			if (node.has_children()) {
				auto children = node.children(ray.sign[node.axis()], n);
				node_stack.push(children.b);
				n = children.a;
			} else {
				for (uint32_t i = node.start_index; i < node.end_index; ++i) {
					if (triangles_[i].intersect_p(ray)) {
						return true;
					}

//					if (triangle::intersect_p(vertices_[itriangles_[i].a].p,
//											  vertices_[itriangles_[i].b].p,
//											  vertices_[itriangles_[i].c].p,
//											  ray)) {
//						return true;
//					}

//					if (ptriangles_[i].intersect_p(ray)) {
//						return true;
//					}
				}

				n = node_stack.pop();
			}
		} else {
			n = node_stack.pop();
		}
	}

	return false;
}

float Tree::opacity(math::Oray& ray, const math::float2& /*bounds*/, Node_stack& node_stack,
					 const material::Materials& materials, const image::texture::sampler::Sampler_2D& sampler) const {
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	float opacity = 0.f;

	math::float2 uv;
	float max_t = ray.max_t;

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.aabb.intersect_p(ray)) {
			if (node.has_children()) {
				auto children = node.children(ray.sign[node.axis()], n);
				node_stack.push(children.b);
				n = children.a;
			} else {
				for (uint32_t i = node.start_index; i < node.end_index; ++i) {
					if (triangles_[i].intersect(ray, uv)) {
						uv = triangles_[i].interpolate_uv(uv);
						opacity += (1.f - opacity) * materials[triangles_[i].material_index]->opacity(uv, sampler);
						if (opacity >= 1.f) {
							return 1.f;
						}
						ray.max_t = max_t;
					}
				}

				n = node_stack.pop();
			}
		} else {
			n = node_stack.pop();
		}
	}

	return opacity;
}

void Tree::interpolate_triangle_data(uint32_t index, math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const {
	triangles_[index].interpolate_data(uv, n, t, tc);

//	triangle::interpolate_data(vertices_[itriangles_[index].a],
//							   vertices_[itriangles_[index].b],
//							   vertices_[itriangles_[index].c],
//							   uv, n, t, tc);

//	dtriangles_[index].interpolate_data(uv, n, t, tc);
}

math::float2 Tree::interpolate_triangle_uv(uint32_t index, math::float2 uv) const {
	return triangles_[index].interpolate_uv(uv);

//	return triangle::interpolate_uv(vertices_[itriangles_[index].a].uv,
//									vertices_[itriangles_[index].b].uv,
//									vertices_[itriangles_[index].c].uv,
//									uv);

//	return triangles_[index].interpolate_uv(uv);
}

uint32_t Tree::triangle_material_index(uint32_t index) const {
	return triangles_[index].material_index;

//	return itriangles_[index].material_index;

//	return dtriangles_[index].material_index;
}

math::float3 Tree::triangle_normal(uint32_t index) const {
	return triangles_[index].normal();

//	return triangle::normal(vertices_[itriangles_[index].a].p,
//							vertices_[itriangles_[index].b].p,
//							vertices_[itriangles_[index].c].p);

//	return ptriangles_[index].normal();
}

void Tree::sample(uint32_t triangle, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const {
	triangles_[triangle].interpolate(math::sample_triangle_uniform(r2), p, n, tc);
}

std::vector<Node>& Tree::allocate_nodes(uint32_t num_nodes) {
	nodes_.resize(num_nodes);
	return nodes_;
}

void Tree::allocate_triangles(uint32_t num_triangles) {
	triangles_.clear();
	triangles_.reserve(num_triangles);
	num_parts_ = 0;

//	itriangles_.clear();
//	triangles_.reserve(num_triangles);

//	ptriangles_.clear();
//	ptriangles_.reserve(num_triangles);

//	dtriangles_.clear();
//	dtriangles_.reserve(num_triangles);
}

void Tree::add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index) {
	triangles_.push_back(Triangle{a, b, c, material_index});
	num_parts_ = std::max(num_parts_, material_index + 1);

//	ptriangles_.push_back(Position_triangle{a.p, b.p, c.p});

//	Data_triangle t;
//	t.a.n = a.n;
//	t.a.t = a.t;
//	t.a.uv = a.uv;
//	t.b.n = b.n;
//	t.b.t = b.t;
//	t.b.uv = b.uv;
//	t.c.n = c.n;
//	t.c.t = c.t;
//	t.c.uv = c.uv;
//	t.material_index = material_index;
//	dtriangles_.push_back(t);
}

//void Tree::add_triangle(const Index_triangle& t) {
//	itriangles_.push_back(t);
//	num_parts_ = std::max(num_parts_, t.material_index + 1);
//}

}}}}




