#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_MT {
	Triangle_MT(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c, uint32_t material_index);

	struct Vertex {
		Vertex(const shape::Vertex& v);

		math::float3 p, n, t;
		math::float2 uv;
	};

	Vertex a, b, c;
	float bitangent_sign;
	uint32_t material_index;

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::float3& p, math::float3& n, math::float2& tc) const;
	void interpolate(math::float2 uv, math::float3& p, math::float2& tc) const;
	void interpolate(math::float2 uv, math::float3& p) const;
	void interpolate_data(math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;

	math::float3 normal() const;

	float area() const;
	float area(const math::float3& scale) const;
};

struct Intersection_vertex_MT {
	math::float3 p;
};

struct Shading_vertex_MT {
	math::float3 n, t;
	math::float2 uv;
	float bitangent_sign;
	uint32_t material_index;
};

bool intersect(const Intersection_vertex_MT& a,
			   const Intersection_vertex_MT& b,
			   const Intersection_vertex_MT& c,
			   math::Oray& ray, math::float2& uv);

bool intersect_p(const Intersection_vertex_MT& a,
				 const Intersection_vertex_MT& b,
				 const Intersection_vertex_MT& c,
				 const math::Oray& ray);

void interpolate_p(const Intersection_vertex_MT& a,
				   const Intersection_vertex_MT& b,
				   const Intersection_vertex_MT& c,
				   math::float2 uv, math::float3& p);

math::float2 interpolate_uv(const Shading_vertex_MT& a,
							const Shading_vertex_MT& b,
							const Shading_vertex_MT& c,
							math::float2 uv);

void interpolate_data(const Shading_vertex_MT& a,
					  const Shading_vertex_MT& b,
					  const Shading_vertex_MT& c,
					  math::float2 uv,
					  math::float3& n, math::float3& t, math::float2& tc);

float area(const Intersection_vertex_MT& a,
		   const Intersection_vertex_MT& b,
		   const Intersection_vertex_MT& c);

float area(const Intersection_vertex_MT& a,
		   const Intersection_vertex_MT& b,
		   const Intersection_vertex_MT& c,
		   const math::float3& scale);

}}}
