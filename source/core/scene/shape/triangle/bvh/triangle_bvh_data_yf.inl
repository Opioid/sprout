#pragma once

#include "triangle_bvh_data_yf.hpp"
#include "scene/shape/triangle/triangle_primitive_yf.inl"
#include "base/math/sampling/sampling.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

Data_YF::~Data_YF() {}

uint32_t Data_YF::num_triangles() const {
    return static_cast<uint32_t>(triangles_.size());
}

bool Data_YF::intersect(uint32_t index, math::Oray& ray, math::float2& uv) const {
    return triangles_[index].intersect(ray, uv);
}

bool Data_YF::intersect_p(uint32_t index, const math::Oray& ray) const {
    return triangles_[index].intersect_p(ray);
}

void Data_YF::interpolate_data(uint32_t index, math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const {
    triangles_[index].interpolate_data(uv, n, t, tc);
}

math::float2 Data_YF::interpolate_uv(uint32_t index, math::float2 uv) const {
    return triangles_[index].interpolate_uv(uv);
}

uint32_t Data_YF::material_index(uint32_t index) const {
    return triangles_[index].material_index;
}

math::float3 Data_YF::normal(uint32_t index) const {
	return triangles_[index].normal();
}

float Data_YF::area(uint32_t index, const math::float3& scale) const {
    return triangles_[index].area(scale);
}

void Data_YF::sample(uint32_t index, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const {
    triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, n, tc);
}

void Data_YF::allocate_triangles(uint32_t num_triangles) {
    triangles_.clear();
    triangles_.reserve(num_triangles);
}

void Data_YF::add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index) {
	triangles_.push_back(Triangle_YF(a, b, c, material_index));
}

}}}}
