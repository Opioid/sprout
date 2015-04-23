#pragma once

#include "scene/shape/node_stack.hpp"
#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace scene { namespace shape { namespace triangle {

struct Intersection;
struct Triangle;

namespace bvh {

struct Node {
	static const uint32_t has_children_flag = 0xFFFFFFFF;

	struct Children {
		uint32_t a, b;
	};

	Children children(uint8_t sign, uint32_t id) const;

	bool has_children() const;
	void set_has_children(bool children);

	void set_right_child(uint32_t offset);

	math::AABB aabb;
	uint32_t start_index;
	uint32_t end_index;
	uint8_t axis;
};

class Tree  {
public:

	const math::AABB& aabb() const;

	bool intersect(math::Oray& ray, const math::float2& bounds, Node_stack& node_stack, Intersection& intersection) const;
	bool intersect_p(const math::Oray& ray, const math::float2& bounds, Node_stack& node_stack) const;

	void interpolate_triangle(uint32_t index, float u, float v, math::float3& n, math::float3& t, math::float2& uv) const;
	uint32_t triangle_material_index(uint32_t index) const;

	std::vector<Node>& allocate_nodes(uint32_t num_nodes);

private:

	bool intersect_node(uint32_t n, math::Oray& ray, Intersection& intersection) const;
	bool intersect_node_p(uint32_t n, const math::Oray& ray) const;

	std::vector<Node> nodes_;

	std::vector<Triangle> triangles_;

	friend class Builder;
};

}}}}
