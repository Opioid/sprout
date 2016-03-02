#pragma once

#include "triangle_bvh_data_interleaved.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "scene/shape/triangle/triangle_primitive_mte.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename Triangle>
Data_interleaved<Triangle>::~Data_interleaved() {}

template<typename Triangle>
uint32_t Data_interleaved<Triangle>::num_triangles() const {
	return static_cast<uint32_t>(triangles_.capacity());
}

template<typename Triangle>
uint32_t Data_interleaved<Triangle>::current_triangle() const {
	return static_cast<uint32_t>(triangles_.size());
}

template<typename Triangle>
bool Data_interleaved<Triangle>::intersect(uint32_t index, math::Oray& ray, math::float2& uv) const {
    return triangles_[index].intersect(ray, uv);
}

template<typename Triangle>
bool Data_interleaved<Triangle>::intersect_p(uint32_t index, const math::Oray& ray) const {
    return triangles_[index].intersect_p(ray);
}

template<typename Triangle>
bool SU_CALLCONV Data_interleaved<Triangle>::intersect(uint32_t index,
													   math::simd::FVector origin, math::simd::FVector direction,
													   float min_t, float max_t,
													   float& out_t, math::float2& uv) {
	return triangles_[index].intersect(origin, direction, min_t, max_t, out_t, uv);
}

template<typename Triangle>
bool SU_CALLCONV Data_interleaved<Triangle>::intersect_p(uint32_t index,
														 math::simd::FVector origin, math::simd::FVector direction,
														 float min_t, float max_t) {
	return triangles_[index].intersect(origin, direction, min_t, max_t);
}

template<typename Triangle>
void Data_interleaved<Triangle>::interpolate_data(uint32_t index, math::float2 uv,
												  math::float3& n, math::float3& t, math::float2& tc) const {
	triangles_[index].interpolate_data(uv, n, t, tc);
}

template<typename Triangle>
math::float2 Data_interleaved<Triangle>::interpolate_uv(uint32_t index, math::float2 uv) const {
    return triangles_[index].interpolate_uv(uv);
}

template<typename Triangle>
float Data_interleaved<Triangle>::bitangent_sign(uint32_t index) const {
	return triangles_[index].bitangent_sign;
}

template<typename Triangle>
uint32_t Data_interleaved<Triangle>::material_index(uint32_t index) const {
    return triangles_[index].material_index;
}

template<typename Triangle>
math::float3 Data_interleaved<Triangle>::normal(uint32_t index) const {
    return triangles_[index].normal();
}

template<typename Triangle>
float Data_interleaved<Triangle>::area(uint32_t index) const {
	return triangles_[index].area();
}

template<typename Triangle>
float Data_interleaved<Triangle>::area(uint32_t index, const math::float3& scale) const {
    return triangles_[index].area(scale);
}

/*
template<typename Triangle>
void Data_interleaved<Triangle>::sample(uint32_t index, math::float2 r2,
									math::float3& p, math::float3& n, math::float2& tc) const {
    triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, n, tc);
}*/

template<typename Triangle>
void Data_interleaved<Triangle>::sample(uint32_t index, math::float2 r2, math::float3& p, math::float2& tc) const {
	triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, tc);
}

/*
template<typename Triangle>
void Data_interleaved<Triangle>::sample(uint32_t index, math::float2 r2, math::float3& p) const {
	triangles_[index].interpolate(math::sample_triangle_uniform(r2), p);
}
*/
template<typename Triangle>
void Data_interleaved<Triangle>::allocate_triangles(uint32_t num_triangles) {
    triangles_.clear();
    triangles_.reserve(num_triangles);
}

template<typename Triangle>
void Data_interleaved<Triangle>::add_triangle(const Vertex& a, const Vertex& b, const Vertex& c,
											  uint32_t material_index) {
	triangles_.push_back(Triangle(a, b, c, material_index));
}

}}}}
