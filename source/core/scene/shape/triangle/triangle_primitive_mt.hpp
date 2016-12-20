#pragma once

#include "scene/shape/shape_vertex.hpp"
#include "base/math/ray.hpp"
#include "base/math/simd/simd_vector.hpp"

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
	float area(float3_p scale) const;
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

bool intersect(const Intersection_vertex_MT& a,
			   const Intersection_vertex_MT& b,
			   const Intersection_vertex_MT& c,
			   math::Ray& ray, float2& uv);

bool intersect_p(const Intersection_vertex_MT& a,
				 const Intersection_vertex_MT& b,
				 const Intersection_vertex_MT& c,
				 const math::Ray& ray);

bool intersect_p(math::simd::FVector origin,
				 math::simd::FVector direction,
				 math::simd::FVector min_t,
				 math::simd::FVector max_t,
				 const Intersection_vertex_MT& a,
				 const Intersection_vertex_MT& b,
				 const Intersection_vertex_MT& c);

void interpolate_p(const Intersection_vertex_MT& a,
				   const Intersection_vertex_MT& b,
				   const Intersection_vertex_MT& c,
				   float2 uv, float3& p);

float2 interpolate_uv(const Shading_vertex_MT& a,
					  const Shading_vertex_MT& b,
					  const Shading_vertex_MT& c,
					  float2 uv);

void interpolate_data(const Shading_vertex_MT& a,
					  const Shading_vertex_MT& b,
					  const Shading_vertex_MT& c,
					  float2 uv,
					  float3& n, float3& t, float2& tc);

float area(const Intersection_vertex_MT& a,
		   const Intersection_vertex_MT& b,
		   const Intersection_vertex_MT& c);

float area(const Intersection_vertex_MT& a,
		   const Intersection_vertex_MT& b,
		   const Intersection_vertex_MT& c,
		   float3_p scale);

struct Shading_vertex_MTC {
	float4 n_u;
	float4 t_v;
};

float2 interpolate_uv(const Shading_vertex_MTC& a,
					  const Shading_vertex_MTC& b,
					  const Shading_vertex_MTC& c,
					  float2 uv);

void interpolate_data(const Shading_vertex_MTC& a,
					  const Shading_vertex_MTC& b,
					  const Shading_vertex_MTC& c,
					  float2 uv,
					  float3& n, float3& t, float2& tc);

}}}
