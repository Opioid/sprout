#pragma once

#include "triangle_bvh_data.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "scene/shape/triangle/triangle_primitive_mte.inl"
#include "base/math/sampling/sampling.inl"
#include "base/memory/align.inl"

//#include <iostream>

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
bool Data<Intersection_triangle, Shading_triangle>::intersect(uint32_t index, math::Oray& ray, math::float2& uv) const {
	return intersection_triangles_[index].intersect(ray, uv);
}

template<typename Intersection_triangle, typename Shading_triangle>
bool Data<Intersection_triangle, Shading_triangle>::intersect_p(uint32_t index, const math::Oray& ray) const {
	return intersection_triangles_[index].intersect_p(ray);
}

template<typename Intersection_triangle, typename Shading_triangle>
bool SU_CALLCONV Data<Intersection_triangle, Shading_triangle>::intersect(uint32_t index,
													   math::simd::FVector origin, math::simd::FVector direction,
													   float min_t, float max_t,
													   float& out_t, math::float2& uv) {
//	return triangles_[index].intersect(origin, direction, min_t, max_t, out_t, uv);
	return false;
}

template<typename Intersection_triangle, typename Shading_triangle>
bool SU_CALLCONV Data<Intersection_triangle, Shading_triangle>::intersect_p(uint32_t index,
														 math::simd::FVector origin, math::simd::FVector direction,
														 float min_t, float max_t) {
//	return triangles_[index].intersect(origin, direction, min_t, max_t);
	return false;
}

template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::interpolate_data(uint32_t index, math::float2 uv,
												  math::vec3& n, math::vec3& t, math::float2& tc) const {
	shading_triangles_[index].interpolate_data(uv, n, t, tc);
}

template<typename Intersection_triangle, typename Shading_triangle>
math::float2 Data<Intersection_triangle, Shading_triangle>::interpolate_uv(uint32_t index, math::float2 uv) const {
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
math::vec3 Data<Intersection_triangle, Shading_triangle>::normal(uint32_t index) const {
	return intersection_triangles_[index].normal();
}

template<typename Intersection_triangle, typename Shading_triangle>
float Data<Intersection_triangle, Shading_triangle>::area(uint32_t index) const {
	return intersection_triangles_[index].area();
}

template<typename Intersection_triangle, typename Shading_triangle>
float Data<Intersection_triangle, Shading_triangle>::area(uint32_t index, const math::vec3& scale) const {
	return intersection_triangles_[index].area(scale);
}

/*
template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::sample(uint32_t index, math::float2 r2,
									math::vec3& p, math::vec3& n, math::float2& tc) const {
    triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, n, tc);
}*/

template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::sample(uint32_t index, math::float2 r2, math::vec3& p, math::float2& tc) const {
	math::float2 uv = math::sample_triangle_uniform(r2);
	intersection_triangles_[index].interpolate(uv, p);
	tc = shading_triangles_[index].interpolate_uv(uv);


//	triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, tc);
}

/*
template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::sample(uint32_t index, math::float2 r2, math::vec3& p) const {
	triangles_[index].interpolate(math::sample_triangle_uniform(r2), p);
}*/

template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::allocate_triangles(uint32_t num_triangles) {
	num_triangles_ = num_triangles;
	current_triangle_ = 0;

	memory::free_aligned(intersection_triangles_);
	memory::free_aligned(shading_triangles_);

//	std::cout << sizeof(Intersection_triangle) << std::endl;
//	std::cout << sizeof(Shading_triangle) << std::endl;

	intersection_triangles_ = memory::allocate_aligned<Intersection_triangle>(num_triangles);
	shading_triangles_      = memory::allocate_aligned<Shading_triangle>(num_triangles);
}

template<typename Intersection_triangle, typename Shading_triangle>
void Data<Intersection_triangle, Shading_triangle>::add_triangle(const Vertex& a, const Vertex& b, const Vertex& c,
											  uint32_t material_index) {
	intersection_triangles_[current_triangle_] = Intersection_triangle(a, b, c);
	shading_triangles_[current_triangle_] = Shading_triangle(a, b, c, material_index);
	++current_triangle_;
}

}}}}
