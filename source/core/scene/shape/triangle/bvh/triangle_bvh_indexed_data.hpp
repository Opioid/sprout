#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape {

struct Vertex;

namespace triangle { namespace bvh {

template<typename Intersection_vertex, typename Shading_vertex>
class Indexed_data {
public:

	Indexed_data();
	~Indexed_data();

	uint32_t num_triangles() const;
	uint32_t current_triangle() const;

	bool intersect(uint32_t index, math::Oray& ray, math::float2& uv) const;

	bool intersect_p(uint32_t index, const math::Oray& ray) const;

	void interpolate_data(uint32_t index, math::float2 uv,
						  math::float3& n, math::float3& t, math::float2& tc) const;

	math::float2 interpolate_uv(uint32_t index, math::float2 uv) const;

	float    bitangent_sign(uint32_t index) const;
	uint32_t material_index(uint32_t index) const;

	math::float3 normal(uint32_t index) const;

	float area(uint32_t index) const;
	float area(uint32_t index, const math::float3& scale) const;

//	void sample(uint32_t index, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const;
	void sample(uint32_t index, math::float2 r2, math::float3& p, math::float2& tc) const;
//	void sample(uint32_t index, math::float2 r2, math::float3& p) const;

	void allocate_triangles(uint32_t num_triangles, const std::vector<Vertex>& vertices);

	void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index, const std::vector<Vertex>& vertices);

	size_t num_bytes() const;

private:

	uint32_t num_triangles_;
	uint32_t current_triangle_;
	uint32_t num_vertices_;

	struct Index_triangle {
		uint32_t a, b, c;
	};

	Index_triangle*      triangles_;
	Intersection_vertex* intersection_vertices_;
	Shading_vertex*		 shading_vertices_;
};

}}}}
