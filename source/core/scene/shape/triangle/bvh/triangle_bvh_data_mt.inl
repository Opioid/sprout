#pragma once

#include "triangle_bvh_data_mt.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

inline Data_MT::~Data_MT() {}

inline uint32_t Data_MT::num_triangles() const {
    return static_cast<uint32_t>(triangles_.size());
}

inline bool Data_MT::intersect(uint32_t index, math::Oray& ray, math::float2& uv) const {
    return triangles_[index].intersect(ray, uv);
}

inline bool Data_MT::intersect_p(uint32_t index, const math::Oray& ray) const {
    return triangles_[index].intersect_p(ray);
}

inline void Data_MT::interpolate_data(uint32_t index, math::float2 uv,
									  math::float3& n, math::float3& t, math::float2& tc) const {
	triangles_[index].interpolate_data(uv, n, t, tc);
}

inline math::float2 Data_MT::interpolate_uv(uint32_t index, math::float2 uv) const {
    return triangles_[index].interpolate_uv(uv);
}

inline float Data_MT::bitangent_sign(uint32_t index) const {
	return triangles_[index].bitangent_sign;
}

inline uint32_t Data_MT::material_index(uint32_t index) const {
    return triangles_[index].material_index;
}

inline math::float3 Data_MT::normal(uint32_t index) const {
    return triangles_[index].normal();
}

inline float Data_MT::area(uint32_t index, const math::float3& scale) const {
    return triangles_[index].area(scale);
}

inline void Data_MT::sample(uint32_t index, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const {
    triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, n, tc);
}

inline void Data_MT::sample(uint32_t index, math::float2 r2, math::float3& p, math::float2& tc) const {
	triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, tc);
}

inline void Data_MT::sample(uint32_t index, math::float2 r2, math::float3& p) const {
	triangles_[index].interpolate(math::sample_triangle_uniform(r2), p);
}

inline void Data_MT::allocate_triangles(uint32_t num_triangles) {
    triangles_.clear();
    triangles_.reserve(num_triangles);
}

inline void Data_MT::add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index) {
	triangles_.push_back(Triangle_MT{
							 Triangle_MT::Vertex(a),
							 Triangle_MT::Vertex(b),
							 Triangle_MT::Vertex(c),
							 a.bitangent_sign, material_index});
}

}}}}
