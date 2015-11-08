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
struct Index_triangle;
struct Triangle;
struct Position_triangle;
struct Data_triangle;

namespace bvh {

struct Node {
	static const uint32_t has_children_flag = 0xFFFFFFFC;

	struct Children {
		uint32_t a, b;
	};

	uint32_t axis() const;
	void set_axis(uint32_t axis);

	Children children(int sign, uint32_t id) const;

	bool has_children() const;
	void set_has_children(bool children);

	void set_right_child(uint32_t offset);

	math::aabb aabb;
	uint32_t start_index;
	uint32_t end_index;
};

template<typename Data>
class Tree  {
public:

    std::vector<Node>& allocate_nodes(uint32_t num_nodes);

	const math::aabb& aabb() const;

	uint32_t num_parts() const;

	uint32_t num_triangles() const;

	bool intersect(math::Oray& ray, Node_stack& node_stack, Intersection& intersection) const;

	bool intersect_p(const math::Oray& ray, Node_stack& node_stack) const;

	float opacity(math::Oray& ray, Node_stack& node_stack,
				  const material::Materials& materials,
				  const image::texture::sampler::Sampler_2D& sampler) const;

	void interpolate_triangle_data(uint32_t index, math::float2 uv,
								   math::float3& n, math::float3& t, math::float2& tc) const;

    math::float2 interpolate_triangle_uv(uint32_t index, math::float2 uv) const;

	float    triangle_bitangent_sign(uint32_t index) const;
    uint32_t triangle_material_index(uint32_t index) const;

	math::float3 triangle_normal(uint32_t index) const;

    float triangle_area(uint32_t index, const math::float3& scale) const;

	void sample(uint32_t index, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const;
	void sample(uint32_t index, math::float2 r2, math::float3& p, math::float2& tc) const;
	void sample(uint32_t index, math::float2 r2, math::float3& p) const;

	void allocate_triangles(uint32_t num_triangles);

	void add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index);

private:

	std::vector<Node> nodes_;

    Data data_;

	uint32_t num_parts_;

	friend class Builder;
};

}}}}
