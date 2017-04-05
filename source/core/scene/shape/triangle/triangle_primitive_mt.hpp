#pragma once

#include "scene/shape/shape_vertex.hpp"
#include "base/math/vector4.hpp"

namespace math { struct Ray; }

namespace scene { namespace shape { namespace triangle {

struct Triangle_MT {
	Triangle_MT(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c,
				float bitangent_sign, uint32_t material_index);

	struct Vertex {
		Vertex(const shape::Vertex& v);

		float3 p, n, t;
		float2 uv;
	};

	Vertex a, b, c;
	float bitangent_sign;
	uint32_t material_index;

	bool intersect(math::Ray& ray, float2& uv) const;
	bool intersect_p(const math::Ray& ray) const;

	void interpolate(float2 uv, float3& p, float3& n, float2& tc) const;
	void interpolate(float2 uv, float3& p, float2& tc) const;
	void interpolate(float2 uv, float3& p) const;
	void interpolate_data(float2 uv, float3& n,
						  float3& t, float2& tc) const;
	float2 interpolate_uv(float2 uv) const;

	float3 normal() const;

	float area() const;
	float area(const float3& scale) const;
};

struct Intersection_vertex_MT {
	float3 p;
};

struct Shading_vertex_MT {
	float3 n, t;
	float2 uv;
	float bitangent_sign;
	uint32_t material_index;
};

struct Shading_vertex_MTC {
	Shading_vertex_MTC(const packed_float3& n, const packed_float3& t, float2 uv);

	float4 n_u;
	float4 t_v;
};

struct alignas(16) Shading_vertex_MTCC {
	Shading_vertex_MTCC(const packed_float3& n, const packed_float3& t, float2 uv);

	short4 n_u;
	short4 t_v;
};

struct Vertex_MTC {
	Vertex_MTC(const packed_float3& p,
			   const packed_float3& n,
			   const packed_float3& t,
			   float2 uv);

	float3 p;
	float4 n_u;
	float4 t_v;
};

}}}
