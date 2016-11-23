#pragma once

#include "triangle_bvh_data.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "scene/shape/triangle/triangle_primitive_mte.inl"
#include "base/math/sampling/sampling.inl"
#include "base/memory/align.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename Intersection_triangle, typename Shading_triangle>
Data<Intersection_triangle, Shading_triangle>::Data() :
	num_triangles_(0), current_triangle_(0),
	intersection_triangles_(nullptr), shading_triangles_(nullptr) {}

template<typename Intersection_triangle, typename Shading_triangle>
Data<Intersection_triangle, Shading_triangle>::~Data() {
	memory::free_aligned(intersection_triangles_);
	memory::free_aligned(shading_triangles_);
}

template<typename Intersection_triangle, typename Shading_triangle>
uint32_t Data<Intersection_triangle, Shading_triangle>::num_triangles() const {
	return num_triangles_;
}

template<typename Intersection_triangle, typename Shading_triangle>
uint32_t Data<Intersection_triangle, Shading_triangle>::current_triangle() const {
	return current_triangle_;
}

template<typename Intersection_triangle, typename Shading_triangle>
bool Data<Intersection_triangle, Shading_triangle>::intersect(uint32_t index, math::Ray& ray, float2& uv) const {
	return intersection_triangles_[index].intersect(ray, uv);
}

template<typename Intersection_triangle, typename Shading_triangle>
bool Data<Intersection_triangle, Shading_triangle>::intersect_p(uint32_t index, const math::Ray& ray) const {
	return intersection_triangles_[index].intersect_p(ray);
}

template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::interpolate_data(
		uint32_t index, float2 uv, float3& n, float3& t, float2& tc) const {
	shading_triangles_[index].interpolate_data(uv, n, t, tc);
}

template<typename Intersection_triangle, typename Shading_triangle>
float2 Data<Intersection_triangle, Shading_triangle>::interpolate_uv(uint32_t index, float2 uv) const {
	return shading_triangles_[index].interpolate_uv(uv);
}

template<typename Intersection_triangle, typename Shading_triangle>
float Data<Intersection_triangle, Shading_triangle>::bitangent_sign(uint32_t index) const {
	return shading_triangles_[index].bitangent_sign;
}

template<typename Intersection_triangle, typename Shading_triangle>
uint32_t Data<Intersection_triangle, Shading_triangle>::material_index(uint32_t index) const {
	return shading_triangles_[index].material_index;
}

template<typename Intersection_triangle, typename Shading_triangle>
float3 Data<Intersection_triangle, Shading_triangle>::normal(uint32_t index) const {
	return intersection_triangles_[index].normal();
}

template<typename Intersection_triangle, typename Shading_triangle>
float Data<Intersection_triangle, Shading_triangle>::area(uint32_t index) const {
	return intersection_triangles_[index].area();
}

template<typename Intersection_triangle, typename Shading_triangle>
float Data<Intersection_triangle, Shading_triangle>::area(uint32_t index, const float3& scale) const {
	return intersection_triangles_[index].area(scale);
}

template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::sample(uint32_t index, float2 r2,
														   float3& p, float2& tc) const {
	float2 uv = math::sample_triangle_uniform(r2);
	intersection_triangles_[index].interpolate(uv, p);
	tc = shading_triangles_[index].interpolate_uv(uv);
}

template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::allocate_triangles(uint32_t num_triangles,
																	   const std::vector<Vertex>& /*vertices*/) {
	num_triangles_ = num_triangles;
	current_triangle_ = 0;

	memory::free_aligned(intersection_triangles_);
	memory::free_aligned(shading_triangles_);

	intersection_triangles_ = memory::allocate_aligned<Intersection_triangle>(num_triangles);
	shading_triangles_      = memory::allocate_aligned<Shading_triangle>(num_triangles);
}

template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::add_triangle(
		uint32_t a, uint32_t b, uint32_t c, uint32_t material_index, const std::vector<Vertex>& vertices) {
	const Vertex& va = vertices[a];
	const Vertex& vb = vertices[b];
	const Vertex& vc = vertices[c];
	intersection_triangles_[current_triangle_] = Intersection_triangle(va, vb, vc);
	shading_triangles_[current_triangle_]	   = Shading_triangle(va, vb, vc, material_index);
	++current_triangle_;
}

template<typename Intersection_triangle, typename Shading_triangle>
size_t Data<Intersection_triangle, Shading_triangle>::num_bytes() const {
	return num_triangles_ * (sizeof(Intersection_triangle) + sizeof(Shading_triangle));
}

}}}}
