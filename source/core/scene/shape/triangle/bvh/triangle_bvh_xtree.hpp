#pragma once

#include "scene/shape/node_stack.hpp"
#include "scene/material/material.hpp"
#include "base/math/vector.hpp"
#include "base/math/ray.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Intersection;
struct Triangle;

namespace bvh {

struct XNode {
	static const uint32_t has_children_flag = 0xFFFFFFFF;

	struct Children {
		uint32_t a, b;
	};

	Children children(uint8_t sign, uint32_t id) const;

	bool has_children() const;
	void set_has_children(bool children);

	void set_right_child(uint32_t offset);

	math::aabb aabb;
	uint32_t start_index;
	uint32_t end_index;
	uint8_t axis;
};

class XTree  {
public:

	const math::aabb& aabb() const;

	uint32_t num_parts() const;
	uint32_t num_triangles() const;

	const std::vector<Triangle>& triangles() const;

	bool intersect(math::Oray& ray, const math::float2& bounds, Node_stack& node_stack, Intersection& intersection) const;
	bool intersect_p(const math::Oray& ray, const math::float2& bounds, Node_stack& node_stack) const;

	float opacity(const math::Oray& ray, const math::float2& bounds, Node_stack& node_stack,
				  const material::Materials& materials, const image::sampler::Sampler_2D& sampler) const;

	void interpolate_triangle_data(uint32_t index, math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const;
	math::float2 interpolate_triangle_uv(uint32_t index, math::float2 uv) const;
	uint32_t triangle_material_index(uint32_t index) const;

	math::float3 triangle_normal(uint32_t index) const;

	void sample(uint32_t triangle, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const;

	std::vector<XNode>& allocate_nodes(uint32_t num_nodes);

	void allocate_triangles(uint32_t num_triangles);

	void add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index);

private:

	bool intersect_node(uint32_t n, math::Oray& ray, Intersection& intersection) const;
	bool intersect_node_p(uint32_t n, const math::Oray& ray) const;

	std::vector<XNode> nodes_;

	std::vector<Triangle> triangles_;

	uint32_t num_parts_;

	friend class XBuilder;
};

}}}}
