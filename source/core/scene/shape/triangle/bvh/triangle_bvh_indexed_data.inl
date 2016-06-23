#pragma once

#include "triangle_bvh_indexed_data.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "scene/shape/triangle/triangle_primitive_mte.inl"
#include "base/math/sampling/sampling.inl"
#include "base/memory/align.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename Intersection_vertex, typename Shading_vertex>
Indexed_data<Intersection_vertex, Shading_vertex>::Indexed_data() :
	num_triangles_(0), current_triangle_(0), num_vertices_(0),
	triangles_(nullptr), intersection_vertices_(nullptr), shading_vertices_(nullptr) {}

template<typename Intersection_vertex, typename Shading_vertex>
Indexed_data<Intersection_vertex, Shading_vertex>::~Indexed_data() {
	memory::free_aligned(triangles_);
	memory::free_aligned(intersection_vertices_);
	memory::free_aligned(shading_vertices_);
}

template<typename Intersection_vertex, typename Shading_vertex>
uint32_t Indexed_data<Intersection_vertex, Shading_vertex>::num_triangles() const {
	return num_triangles_;
}

template<typename Intersection_vertex, typename Shading_vertex>
uint32_t Indexed_data<Intersection_vertex, Shading_vertex>::current_triangle() const {
	return current_triangle_;
}

template<typename Intersection_vertex, typename Shading_vertex>
bool Indexed_data<Intersection_vertex, Shading_vertex>::intersect(uint32_t index,
																  math::Oray& ray,
																  float2& uv) const {
	const auto& t = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[t.a];
	const Intersection_vertex& b = intersection_vertices_[t.b];
	const Intersection_vertex& c = intersection_vertices_[t.c];

	return triangle::intersect(a, b, c, ray, uv);
}

template<typename Intersection_vertex, typename Shading_vertex>
bool Indexed_data<Intersection_vertex, Shading_vertex>::intersect_p(uint32_t index,
																	const math::Oray& ray) const {
	const auto& tri = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[tri.a];
	const Intersection_vertex& b = intersection_vertices_[tri.b];
	const Intersection_vertex& c = intersection_vertices_[tri.c];

	return triangle::intersect_p(a, b, c, ray);
}

template<typename Intersection_vertex, typename Shading_vertex>
void Indexed_data<Intersection_vertex, Shading_vertex>::interpolate_data(uint32_t index,
																		 float2 uv,
																		 float3& n,
																		 float3& t,
																		 float2& tc) const {
	const auto& tri = triangles_[index];
	const Shading_vertex& a = shading_vertices_[tri.a];
	const Shading_vertex& b = shading_vertices_[tri.b];
	const Shading_vertex& c = shading_vertices_[tri.c];

	triangle::interpolate_data(a, b, c, uv, n, t, tc);
}

template<typename Intersection_vertex, typename Shading_vertex>
float2 Indexed_data<Intersection_vertex, Shading_vertex>::interpolate_uv(
		uint32_t index, float2 uv) const {
	const auto& tri = triangles_[index];
	const Shading_vertex& sa = shading_vertices_[tri.a];
	const Shading_vertex& sb = shading_vertices_[tri.b];
	const Shading_vertex& sc = shading_vertices_[tri.c];

	return triangle::interpolate_uv(sa, sb, sc, uv);
}

template<typename Intersection_vertex, typename Shading_vertex>
float Indexed_data<Intersection_vertex, Shading_vertex>::bitangent_sign(uint32_t index) const {
	return shading_vertices_[triangles_[index].a].bitangent_sign;
}

template<typename Intersection_vertex, typename Shading_vertex>
uint32_t Indexed_data<Intersection_vertex, Shading_vertex>::material_index(uint32_t index) const {
	return shading_vertices_[triangles_[index].a].material_index;
}

template<typename Intersection_vertex, typename Shading_vertex>
float3 Indexed_data<Intersection_vertex, Shading_vertex>::normal(uint32_t index) const {
	const auto& tri = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[tri.a];
	const Intersection_vertex& b = intersection_vertices_[tri.b];
	const Intersection_vertex& c = intersection_vertices_[tri.c];

	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;
	return math::normalized(math::cross(e1, e2));
}

template<typename Intersection_vertex, typename Shading_vertex>
float Indexed_data<Intersection_vertex, Shading_vertex>::area(uint32_t index) const {
	const auto& tri = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[tri.a];
	const Intersection_vertex& b = intersection_vertices_[tri.b];
	const Intersection_vertex& c = intersection_vertices_[tri.c];

	return triangle::area(a, b, c);
}

template<typename Intersection_vertex, typename Shading_vertex>
float Indexed_data<Intersection_vertex, Shading_vertex>::area(uint32_t index,
															  const float3& scale) const {
	const auto& tri = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[tri.a];
	const Intersection_vertex& b = intersection_vertices_[tri.b];
	const Intersection_vertex& c = intersection_vertices_[tri.c];

	return triangle::area(a, b, c, scale);
}

template<typename Intersection_vertex, typename Shading_vertex>
void Indexed_data<Intersection_vertex, Shading_vertex>::sample(uint32_t index,
															   float2 r2,
															   float3& p,
															   float2& tc) const {
	float2 uv = math::sample_triangle_uniform(r2);

	const auto& tri = triangles_[index];
	const Intersection_vertex& ia = intersection_vertices_[tri.a];
	const Intersection_vertex& ib = intersection_vertices_[tri.b];
	const Intersection_vertex& ic = intersection_vertices_[tri.c];

	triangle::interpolate_p(ia, ib, ic, uv, p);

	const Shading_vertex& sa = shading_vertices_[tri.a];
	const Shading_vertex& sb = shading_vertices_[tri.b];
	const Shading_vertex& sc = shading_vertices_[tri.c];

	tc = triangle::interpolate_uv(sa, sb, sc, uv);
}

template<typename Intersection_vertex, typename Shading_vertex>
void Indexed_data<Intersection_vertex, Shading_vertex>::allocate_triangles(
		uint32_t num_triangles, const std::vector<Vertex>& vertices) {
	num_triangles_ = num_triangles;
	current_triangle_ = 0;
	num_vertices_ = static_cast<uint32_t>(vertices.size());

	memory::free_aligned(triangles_);
	memory::free_aligned(intersection_vertices_);
	memory::free_aligned(shading_vertices_);

	triangles_ = memory::allocate_aligned<Index_triangle>(num_triangles);

	intersection_vertices_ = memory::allocate_aligned<Intersection_vertex>(num_vertices_);
	shading_vertices_      = memory::allocate_aligned<Shading_vertex>(num_vertices_);

	for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
		intersection_vertices_[i].p = float3(vertices[i].p);

		shading_vertices_[i].n  = float3(vertices[i].n);
		shading_vertices_[i].t  = float3(vertices[i].t);
		shading_vertices_[i].uv = vertices[i].uv;
		shading_vertices_[i].bitangent_sign = vertices[i].bitangent_sign;
	}
}

template<typename Intersection_vertex, typename Shading_vertex>
void Indexed_data<Intersection_vertex, Shading_vertex>::add_triangle(
		uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
		const std::vector<Vertex>& /*vertices*/) {
	triangles_[current_triangle_] = Index_triangle{a, b, c};
	++current_triangle_;

	shading_vertices_[a].material_index = material_index;
	shading_vertices_[b].material_index = material_index;
	shading_vertices_[c].material_index = material_index;
}

template<typename Intersection_vertex, typename Shading_vertex>
size_t Indexed_data<Intersection_vertex, Shading_vertex>::num_bytes() const {
	return num_triangles_ * sizeof(Index_triangle) +
		   num_vertices_ * (sizeof(Intersection_vertex) + sizeof(Shading_vertex));
}

template<typename Intersection_vertex, typename Shading_vertex>
Indexed_data1<Intersection_vertex, Shading_vertex>::Indexed_data1() :
	num_triangles_(0), current_triangle_(0), num_vertices_(0),
	triangles_(nullptr), intersection_vertices_(nullptr), shading_vertices_(nullptr) {}

template<typename Intersection_vertex, typename Shading_vertex>
Indexed_data1<Intersection_vertex, Shading_vertex>::~Indexed_data1() {
	memory::free_aligned(triangles_);
	memory::free_aligned(intersection_vertices_);
	memory::free_aligned(shading_vertices_);
}

template<typename Intersection_vertex, typename Shading_vertex>
uint32_t Indexed_data1<Intersection_vertex, Shading_vertex>::num_triangles() const {
	return num_triangles_;
}

template<typename Intersection_vertex, typename Shading_vertex>
uint32_t Indexed_data1<Intersection_vertex, Shading_vertex>::current_triangle() const {
	return current_triangle_;
}

template<typename Intersection_vertex, typename Shading_vertex>
bool Indexed_data1<Intersection_vertex, Shading_vertex>::intersect(uint32_t index,
																  math::Oray& ray,
																  float2& uv) const {
	const auto& t = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[t.a];
	const Intersection_vertex& b = intersection_vertices_[t.b];
	const Intersection_vertex& c = intersection_vertices_[t.c];

	return triangle::intersect(a, b, c, ray, uv);
}

template<typename Intersection_vertex, typename Shading_vertex>
bool Indexed_data1<Intersection_vertex, Shading_vertex>::intersect_p(uint32_t index,
																	 const math::Oray& ray) const {
	const auto& tri = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[tri.a];
	const Intersection_vertex& b = intersection_vertices_[tri.b];
	const Intersection_vertex& c = intersection_vertices_[tri.c];

	return triangle::intersect_p(a, b, c, ray);
}

template<typename Intersection_vertex, typename Shading_vertex>
void Indexed_data1<Intersection_vertex, Shading_vertex>::interpolate_data(uint32_t index,
																		  float2 uv,
																		  float3& n,
																		  float3& t,
																		  float2& tc) const {
	const auto& tri = triangles_[index];
	const Shading_vertex& a = shading_vertices_[tri.a];
	const Shading_vertex& b = shading_vertices_[tri.b];
	const Shading_vertex& c = shading_vertices_[tri.c];

	triangle::interpolate_data(a, b, c, uv, n, t, tc);
}

template<typename Intersection_vertex, typename Shading_vertex>
float2 Indexed_data1<Intersection_vertex, Shading_vertex>::interpolate_uv(
		uint32_t index, float2 uv) const {
	const auto& tri = triangles_[index];
	const Shading_vertex& sa = shading_vertices_[tri.a];
	const Shading_vertex& sb = shading_vertices_[tri.b];
	const Shading_vertex& sc = shading_vertices_[tri.c];

	return triangle::interpolate_uv(sa, sb, sc, uv);
}

template<typename Intersection_vertex, typename Shading_vertex>
float Indexed_data1<Intersection_vertex, Shading_vertex>::bitangent_sign(uint32_t index) const {
	constexpr float sign[2] = { 1.f, -1.f };

	return sign[(Index_triangle::BTS_mask & triangles_[index].bts_material_index) >> 31];
}

template<typename Intersection_vertex, typename Shading_vertex>
uint32_t Indexed_data1<Intersection_vertex, Shading_vertex>::material_index(uint32_t index) const {
	return Index_triangle::Material_index_mask & triangles_[index].bts_material_index;
}

template<typename Intersection_vertex, typename Shading_vertex>
float3 Indexed_data1<Intersection_vertex, Shading_vertex>::normal(uint32_t index) const {
	const auto& tri = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[tri.a];
	const Intersection_vertex& b = intersection_vertices_[tri.b];
	const Intersection_vertex& c = intersection_vertices_[tri.c];

	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;
	return math::normalized(math::cross(e1, e2));
}

template<typename Intersection_vertex, typename Shading_vertex>
float Indexed_data1<Intersection_vertex, Shading_vertex>::area(uint32_t index) const {
	const auto& tri = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[tri.a];
	const Intersection_vertex& b = intersection_vertices_[tri.b];
	const Intersection_vertex& c = intersection_vertices_[tri.c];

	return triangle::area(a, b, c);
}

template<typename Intersection_vertex, typename Shading_vertex>
float Indexed_data1<Intersection_vertex, Shading_vertex>::area(uint32_t index,
															   const float3& scale) const {
	const auto& tri = triangles_[index];
	const Intersection_vertex& a = intersection_vertices_[tri.a];
	const Intersection_vertex& b = intersection_vertices_[tri.b];
	const Intersection_vertex& c = intersection_vertices_[tri.c];

	return triangle::area(a, b, c, scale);
}

template<typename Intersection_vertex, typename Shading_vertex>
void Indexed_data1<Intersection_vertex, Shading_vertex>::sample(uint32_t index,
																float2 r2,
																float3& p,
																float2& tc) const {
	float2 uv = math::sample_triangle_uniform(r2);

	const auto& tri = triangles_[index];
	const Intersection_vertex& ia = intersection_vertices_[tri.a];
	const Intersection_vertex& ib = intersection_vertices_[tri.b];
	const Intersection_vertex& ic = intersection_vertices_[tri.c];

	triangle::interpolate_p(ia, ib, ic, uv, p);

	const Shading_vertex& sa = shading_vertices_[tri.a];
	const Shading_vertex& sb = shading_vertices_[tri.b];
	const Shading_vertex& sc = shading_vertices_[tri.c];

	tc = triangle::interpolate_uv(sa, sb, sc, uv);
}

template<typename Intersection_vertex, typename Shading_vertex>
void Indexed_data1<Intersection_vertex, Shading_vertex>::allocate_triangles(
		uint32_t num_triangles, const std::vector<Vertex>& vertices) {
	num_triangles_ = num_triangles;
	current_triangle_ = 0;
	num_vertices_ = static_cast<uint32_t>(vertices.size());

	memory::free_aligned(triangles_);
	memory::free_aligned(intersection_vertices_);
	memory::free_aligned(shading_vertices_);

	triangles_ = memory::allocate_aligned<Index_triangle>(num_triangles);

	intersection_vertices_ = memory::allocate_aligned<Intersection_vertex>(num_vertices_);
	shading_vertices_      = memory::allocate_aligned<Shading_vertex>(num_vertices_);

	for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
		intersection_vertices_[i].p = float3(vertices[i].p);

		shading_vertices_[i].n_u = float4(vertices[i].n, vertices[i].uv.x);
		shading_vertices_[i].t_v = float4(vertices[i].t, vertices[i].uv.y);

		// Not too happy about handling degenerate tangents here (only one very special case even)
		if (0.f == shading_vertices_[i].t_v.x
		&&  0.f == shading_vertices_[i].t_v.y
		&&  0.f == shading_vertices_[i].t_v.z) {
			shading_vertices_[i].t_v.xyz = math::tangent(shading_vertices_[i].n_u.xyz);
		}
	}
}

template<typename Intersection_vertex, typename Shading_vertex>
void Indexed_data1<Intersection_vertex, Shading_vertex>::add_triangle(
		uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
		const std::vector<Vertex>& vertices) {
	float bitanget_sign = 1.f;

	if ((vertices[a].bitangent_sign < 0.f && vertices[b].bitangent_sign < 0.f)
	||  (vertices[b].bitangent_sign < 0.f && vertices[c].bitangent_sign < 0.f)
	||  (vertices[c].bitangent_sign < 0.f && vertices[a].bitangent_sign < 0.f)) {
		bitanget_sign = -1.f;
	}

	triangles_[current_triangle_] = Index_triangle(a, b, c, material_index, bitanget_sign);
	++current_triangle_;
}

template<typename Intersection_vertex, typename Shading_vertex>
size_t Indexed_data1<Intersection_vertex, Shading_vertex>::num_bytes() const {
	return num_triangles_ * sizeof(Index_triangle) +
		   num_vertices_ * (sizeof(Intersection_vertex) + sizeof(Shading_vertex));
}

template<typename Intersection_vertex, typename Shading_vertex>
Indexed_data1<Intersection_vertex, Shading_vertex>::
Index_triangle::Index_triangle(uint32_t a, uint32_t b, uint32_t c,
							   uint32_t material_index, float bitangent_sign) :
	a(a),
	b(b),
	c(c),
	bts_material_index(bitangent_sign < 0.f ? BTS_mask | material_index : material_index) {}

}}}}
