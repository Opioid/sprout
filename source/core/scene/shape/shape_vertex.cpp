#include "shape_vertex.hpp"
#include "base/math/vector3.inl"

namespace scene::shape {

size_t Vertex::unpadded_size() noexcept {
    return 3 * 4 + 3 * 4 + 3 * 4 + 2 * 4 + 1;
}

Vertex_stream::~Vertex_stream() noexcept {}

Vertex_stream::Vertex_stream(uint32_t num_vertices) noexcept : num_vertices_(num_vertices) {}

uint32_t Vertex_stream::num_vertices() const noexcept {
    return num_vertices_;
}

Vertex_stream_interleaved::Vertex_stream_interleaved(uint32_t      num_vertices,
                                                     Vertex const* vertices) noexcept
    : Vertex_stream(num_vertices), vertices_(vertices) {}

void Vertex_stream_interleaved::release() noexcept {
    delete[] vertices_;
}

float3 Vertex_stream_interleaved::p(uint32_t i) const noexcept {
    return float3(vertices_[i].p);
}

float3 Vertex_stream_interleaved::n(uint32_t i) const noexcept {
    return float3(vertices_[i].n);
}

float3 Vertex_stream_interleaved::t(uint32_t i) const noexcept {
    return float3(vertices_[i].t);
}

float2 Vertex_stream_interleaved::uv(uint32_t i) const noexcept {
    return vertices_[i].uv;
}

uint8_t Vertex_stream_interleaved::bitangent_sign(uint32_t i) const noexcept {
    return vertices_[i].bitangent_sign;
}

Vertex_stream_separate::Vertex_stream_separate(uint32_t num_vertices, packed_float3 const* p,
                                               packed_float3 const* n, packed_float3 const* t,
                                               float2 const* uv, uint8_t const* bts) noexcept
    : Vertex_stream(num_vertices), p_(p), n_(n), t_(t), uv_(uv), bts_(bts) {}

void Vertex_stream_separate::release() noexcept {
    delete[] p_;
    delete[] n_;
    delete[] t_;
    delete[] uv_;
    delete[] bts_;
}

float3 Vertex_stream_separate::p(uint32_t i) const noexcept {
    return float3(p_[i]);
}

float3 Vertex_stream_separate::n(uint32_t i) const noexcept {
    return float3(n_[i]);
}

float3 Vertex_stream_separate::t(uint32_t i) const noexcept {
    return float3(t_[i]);
}

float2 Vertex_stream_separate::uv(uint32_t i) const noexcept {
    return uv_[i];
}

uint8_t Vertex_stream_separate::bitangent_sign(uint32_t i) const noexcept {
    return bts_[i];
}

Vertex_stream_separate_compact::Vertex_stream_separate_compact(uint32_t             num_vertices,
                                                               packed_float3 const* p,
                                                               packed_float3 const* n) noexcept
    : Vertex_stream(num_vertices), p_(p), n_(n) {}

void Vertex_stream_separate_compact::release() noexcept {
    delete[] p_;
    delete[] n_;
}

float3 Vertex_stream_separate_compact::p(uint32_t i) const noexcept {
    return float3(p_[i]);
}

float3 Vertex_stream_separate_compact::n(uint32_t i) const noexcept {
    return float3(n_[i]);
}

float3 Vertex_stream_separate_compact::t(uint32_t i) const noexcept {
    return tangent(float3(n_[i]));
}

float2 Vertex_stream_separate_compact::uv(uint32_t /*i*/) const noexcept {
    return float2(0.f);
}

uint8_t Vertex_stream_separate_compact::bitangent_sign(uint32_t /*i*/) const noexcept {
    return 0;
}

}  // namespace scene::shape
