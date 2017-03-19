#pragma once

#include "triangle_bvh_indexed_data.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename IV, typename SV>
Indexed_data<IV, SV>::Indexed_data() :
	num_triangles_(0), current_triangle_(0), num_vertices_(0),
	triangles_(nullptr), intersection_vertices_(nullptr), shading_vertices_(nullptr) {}

template<typename IV, typename SV>
Indexed_data<IV, SV>::~Indexed_data() {
	memory::free_aligned(shading_vertices_);
	memory::free_aligned(intersection_vertices_);
	memory::free_aligned(triangles_);
}

template<typename IV, typename SV>
uint32_t Indexed_data<IV, SV>::num_triangles() const {
	return num_triangles_;
}

template<typename IV, typename SV>
uint32_t Indexed_data<IV, SV>::current_triangle() const {
	return current_triangle_;
}

template<typename IV, typename SV>
bool Indexed_data<IV, SV>::intersect(uint32_t index, math::Ray& ray, float2& uv) const {
	const auto& t = triangles_[index];
	const IV& a = intersection_vertices_[t.a];
	const IV& b = intersection_vertices_[t.b];
	const IV& c = intersection_vertices_[t.c];

	return triangle::intersect(a, b, c, ray, uv);
}

template<typename IV, typename SV>
bool Indexed_data<IV, SV>::intersect_p(uint32_t index, const math::Ray& ray) const {
	const auto& tri = triangles_[index];
	const IV& a = intersection_vertices_[tri.a];
	const IV& b = intersection_vertices_[tri.b];
	const IV& c = intersection_vertices_[tri.c];

	return triangle::intersect_p(a, b, c, ray);
}

template<typename IV, typename SV>
bool Indexed_data<IV, SV>::intersect(math::simd::FVector origin,
									 math::simd::FVector direction,
									 math::simd::FVector min_t,
									 math::simd::Vector& max_t,
									 math::simd::Vector& u,
									 math::simd::Vector& v,
									 uint32_t index) const {
	const auto& tri = triangles_[index];
	const IV& a = intersection_vertices_[tri.a];
	const IV& b = intersection_vertices_[tri.b];
	const IV& c = intersection_vertices_[tri.c];

	return triangle::intersect(origin, direction, min_t, max_t, u, v, a, b, c);
}

template<typename IV, typename SV>
bool Indexed_data<IV, SV>::intersect_p(math::simd::FVector origin,
									   math::simd::FVector direction,
									   math::simd::FVector min_t,
									   math::simd::FVector max_t,
									   uint32_t index) const {
	const auto& tri = triangles_[index];
	const IV& a = intersection_vertices_[tri.a];
	const IV& b = intersection_vertices_[tri.b];
	const IV& c = intersection_vertices_[tri.c];

	return triangle::intersect_p(origin, direction, min_t, max_t, a, b, c);
}

template<typename IV, typename SV>
void Indexed_data<IV, SV>::interpolate_data(uint32_t index, float2 uv,
											float3& n, float3& t, float2& tc) const {
	const auto& tri = triangles_[index];
	const SV& a = shading_vertices_[tri.a];
	const SV& b = shading_vertices_[tri.b];
	const SV& c = shading_vertices_[tri.c];

	triangle::interpolate_data(a, b, c, uv, n, t, tc);
}

template<typename IV, typename SV>
float2 Indexed_data<IV, SV>::interpolate_uv(uint32_t index, float2 uv) const {
	const auto& tri = triangles_[index];
	const SV& sa = shading_vertices_[tri.a];
	const SV& sb = shading_vertices_[tri.b];
	const SV& sc = shading_vertices_[tri.c];

	return triangle::interpolate_uv(sa, sb, sc, uv);
}

template<typename IV, typename SV>
float2 Indexed_data<IV, SV>::interpolate_uv(math::simd::FVector u,
											math::simd::FVector v,
											uint32_t index) const {
	const auto& tri = triangles_[index];
	const SV& sa = shading_vertices_[tri.a];
	const SV& sb = shading_vertices_[tri.b];
	const SV& sc = shading_vertices_[tri.c];

	return triangle::interpolate_uv(sa, sb, sc, u, v);
}

template<typename IV, typename SV>
float Indexed_data<IV, SV>::bitangent_sign(uint32_t index) const {
	const float signs[2] = { 1.f, -1.f };

	return signs[(Index_triangle::BTS_mask & triangles_[index].bts_material_index) >> 31];
}

template<typename IV, typename SV>
uint32_t Indexed_data<IV, SV>::material_index(uint32_t index) const {
	return Index_triangle::Material_index_mask & triangles_[index].bts_material_index;
}

template<typename IV, typename SV>
float3 Indexed_data<IV, SV>::normal(uint32_t index) const {
	const auto& tri = triangles_[index];
	const IV& a = intersection_vertices_[tri.a];
	const IV& b = intersection_vertices_[tri.b];
	const IV& c = intersection_vertices_[tri.c];

	const float3 e1 = b.p - a.p;
	const float3 e2 = c.p - a.p;
	return math::normalized(math::cross(e1, e2));
}

template<typename IV, typename SV>
float Indexed_data<IV, SV>::area(uint32_t index) const {
	const auto& tri = triangles_[index];
	const IV& a = intersection_vertices_[tri.a];
	const IV& b = intersection_vertices_[tri.b];
	const IV& c = intersection_vertices_[tri.c];

	return triangle::area(a, b, c);
}

template<typename IV, typename SV>
float Indexed_data<IV, SV>::area(uint32_t index, float3_p scale) const {
	const auto& tri = triangles_[index];
	const IV& a = intersection_vertices_[tri.a];
	const IV& b = intersection_vertices_[tri.b];
	const IV& c = intersection_vertices_[tri.c];

	return triangle::area(a, b, c, scale);
}

template<typename IV, typename SV>
void Indexed_data<IV, SV>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
	float2 uv = math::sample_triangle_uniform(r2);

	const auto& tri = triangles_[index];
	const IV& ia = intersection_vertices_[tri.a];
	const IV& ib = intersection_vertices_[tri.b];
	const IV& ic = intersection_vertices_[tri.c];

	triangle::interpolate_p(ia, ib, ic, uv, p);

	const SV& sa = shading_vertices_[tri.a];
	const SV& sb = shading_vertices_[tri.b];
	const SV& sc = shading_vertices_[tri.c];

	tc = triangle::interpolate_uv(sa, sb, sc, uv);
}

template<typename IV, typename SV>
void Indexed_data<IV, SV>::allocate_triangles(uint32_t num_triangles, const Vertices& vertices) {
	uint32_t num_vertices = static_cast<uint32_t>(vertices.size());

	if (num_triangles != num_triangles_ || num_vertices != num_vertices_) {
		num_triangles_ = num_triangles;
		num_vertices_  = num_vertices;

		memory::free_aligned(shading_vertices_);
		memory::free_aligned(intersection_vertices_);
		memory::free_aligned(triangles_);

		triangles_			   = memory::allocate_aligned<Index_triangle>(num_triangles);
		intersection_vertices_ = memory::allocate_aligned<IV>(num_vertices);
		shading_vertices_      = memory::allocate_aligned<SV>(num_vertices);
	}

	for (uint32_t i = 0; i < num_vertices; ++i) {
		intersection_vertices_[i].p = float3(vertices[i].p);

		shading_vertices_[i] = SV(vertices[i].n, vertices[i].t, vertices[i].uv);
	}

	current_triangle_ = 0;
}

template<typename IV, typename SV>
void Indexed_data<IV, SV>::add_triangle(uint32_t a, uint32_t b, uint32_t c,
										uint32_t material_index, const Vertices& vertices) {
	float bitanget_sign = 1.f;

	if ((vertices[a].bitangent_sign < 0.f && vertices[b].bitangent_sign < 0.f)
	||  (vertices[b].bitangent_sign < 0.f && vertices[c].bitangent_sign < 0.f)
	||  (vertices[c].bitangent_sign < 0.f && vertices[a].bitangent_sign < 0.f)) {
		bitanget_sign = -1.f;
	}

	triangles_[current_triangle_] = Index_triangle(a, b, c, bitanget_sign, material_index);
	++current_triangle_;
}

template<typename IV, typename SV>
size_t Indexed_data<IV, SV>::num_bytes() const {
	return sizeof(*this) +
		   num_triangles_ * sizeof(Index_triangle) +
		   num_vertices_ * (sizeof(IV) + sizeof(SV));
}

template<typename IV, typename SV>
Indexed_data<IV, SV>::
Index_triangle::Index_triangle(uint32_t a, uint32_t b, uint32_t c,
							   float bitangent_sign, uint32_t material_index) :
	a(a), b(b), c(c),
	bts_material_index(bitangent_sign < 0.f ? BTS_mask | material_index : material_index) {}

template<typename V>
Indexed_data_interleaved<V>::Indexed_data_interleaved() :
	num_triangles_(0), current_triangle_(0), num_vertices_(0),
	triangles_(nullptr), vertices_(nullptr) {}

template<typename V>
Indexed_data_interleaved<V>::~Indexed_data_interleaved() {
	memory::free_aligned(vertices_);
	memory::free_aligned(triangles_);
}

template<typename V>
uint32_t Indexed_data_interleaved<V>::num_triangles() const {
	return num_triangles_;
}

template<typename V>
uint32_t Indexed_data_interleaved<V>::current_triangle() const {
	return current_triangle_;
}

template<typename V>
bool Indexed_data_interleaved<V>::intersect(uint32_t index, math::Ray& ray, float2& uv) const {
	const auto& t = triangles_[index];
	const V& a = vertices_[t.a];
	const V& b = vertices_[t.b];
	const V& c = vertices_[t.c];

	return triangle::intersect(a, b, c, ray, uv);
}

template<typename V>
bool Indexed_data_interleaved<V>::intersect_p(uint32_t index, const math::Ray& ray) const {
	const auto& tri = triangles_[index];
	const V& a = vertices_[tri.a];
	const V& b = vertices_[tri.b];
	const V& c = vertices_[tri.c];

	return triangle::intersect_p(a, b, c, ray);
}

template<typename V>
void Indexed_data_interleaved<V>::interpolate_data(uint32_t index, float2 uv,
												   float3& n, float3& t, float2& tc) const {
	const auto& tri = triangles_[index];
	const V& a = vertices_[tri.a];
	const V& b = vertices_[tri.b];
	const V& c = vertices_[tri.c];

	triangle::interpolate_data(a, b, c, uv, n, t, tc);
}

template<typename V>
float2 Indexed_data_interleaved<V>::interpolate_uv(uint32_t index, float2 uv) const {
	const auto& tri = triangles_[index];
	const V& sa = vertices_[tri.a];
	const V& sb = vertices_[tri.b];
	const V& sc = vertices_[tri.c];

	return triangle::interpolate_uv(sa, sb, sc, uv);
}

template<typename V>
float Indexed_data_interleaved<V>::bitangent_sign(uint32_t index) const {
	constexpr float signs[2] = { 1.f, -1.f };

	return signs[(Index_triangle::BTS_mask & triangles_[index].bts_material_index) >> 31];
}

template<typename V>
uint32_t Indexed_data_interleaved<V>::material_index(uint32_t index) const {
	return Index_triangle::Material_index_mask & triangles_[index].bts_material_index;
}

template<typename V>
float3 Indexed_data_interleaved<V>::normal(uint32_t index) const {
	const auto& tri = triangles_[index];
	const V& a = vertices_[tri.a];
	const V& b = vertices_[tri.b];
	const V& c = vertices_[tri.c];

	float3 e1 = b.p - a.p;
	float3 e2 = c.p - a.p;
	return math::normalized(math::cross(e1, e2));
}

template<typename V>
float Indexed_data_interleaved<V>::area(uint32_t index) const {
	const auto& tri = triangles_[index];
	const V& a = vertices_[tri.a];
	const V& b = vertices_[tri.b];
	const V& c = vertices_[tri.c];

	return triangle::area(a, b, c);
}

template<typename V>
float Indexed_data_interleaved<V>::area(uint32_t index, float3_p scale) const {
	const auto& tri = triangles_[index];
	const V& a = vertices_[tri.a];
	const V& b = vertices_[tri.b];
	const V& c = vertices_[tri.c];

	return triangle::area(a, b, c, scale);
}

template<typename V>
void Indexed_data_interleaved<V>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
	float2 uv = math::sample_triangle_uniform(r2);

	const auto& tri = triangles_[index];
	const V& ia = vertices_[tri.a];
	const V& ib = vertices_[tri.b];
	const V& ic = vertices_[tri.c];

	triangle::interpolate_p_uv(ia, ib, ic, uv, p, tc);
}

template<typename V>
void Indexed_data_interleaved<V>::allocate_triangles(uint32_t num_triangles,
													 const Vertices& vertices) {
	num_triangles_ = num_triangles;
	current_triangle_ = 0;
	num_vertices_ = static_cast<uint32_t>(vertices.size());

	memory::free_aligned(vertices_);
	memory::free_aligned(triangles_);

	triangles_ = memory::allocate_aligned<Index_triangle>(num_triangles);
	vertices_  = memory::allocate_aligned<V>(num_vertices_);

	for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
		vertices_[i] = V(vertices[i].p, vertices[i].n, vertices[i].t, vertices[i].uv);
	}
}

template<typename V>
void Indexed_data_interleaved<V>::add_triangle(uint32_t a, uint32_t b, uint32_t c,
											   uint32_t material_index, const Vertices& vertices) {
	float bitanget_sign = 1.f;

	if ((vertices[a].bitangent_sign < 0.f && vertices[b].bitangent_sign < 0.f)
	||  (vertices[b].bitangent_sign < 0.f && vertices[c].bitangent_sign < 0.f)
	||  (vertices[c].bitangent_sign < 0.f && vertices[a].bitangent_sign < 0.f)) {
		bitanget_sign = -1.f;
	}

	triangles_[current_triangle_] = Index_triangle(a, b, c, bitanget_sign, material_index);
	++current_triangle_;
}

template<typename V>
size_t Indexed_data_interleaved<V>::num_bytes() const {
	return sizeof(*this) +
		   num_triangles_ * sizeof(Index_triangle) +
		   num_vertices_ * sizeof(V);
}

template<typename V>
Indexed_data_interleaved<V>::
Index_triangle::Index_triangle(uint32_t a, uint32_t b, uint32_t c,
							   float bitangent_sign, uint32_t material_index) :
	a(a), b(b), c(c),
	bts_material_index(bitangent_sign < 0.f ? BTS_mask | material_index : material_index) {}

}}}}
