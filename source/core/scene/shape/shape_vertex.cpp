#include "shape_vertex.hpp"

namespace scene::shape {

size_t Vertex::unpadded_size() noexcept {
    return 3 * 4 + 3 * 4 + 3 * 4 + 2 * 4 + 1;
}

Vertex_stream::~Vertex_stream() noexcept {}

Vertex_stream::Vertex_stream(uint32_t num_vertices) noexcept : num_vertices_(num_vertices) {}

uint32_t Vertex_stream::num_vertices() const noexcept {
    return num_vertices_;
}

Vertex_stream_interleaved::Vertex_stream_interleaved(uint32_t num_vertices, Vertex const* vertices)
    : Vertex_stream(num_vertices), vertices_(vertices) {}

void Vertex_stream_interleaved::release() noexcept {
    delete[] vertices_;
}

packed_float3 Vertex_stream_interleaved::p(uint32_t i) const noexcept {
    return vertices_[i].p;
}

packed_float3 Vertex_stream_interleaved::n(uint32_t i) const noexcept {
    return vertices_[i].n;
}

packed_float3 Vertex_stream_interleaved::t(uint32_t i) const noexcept {
    return vertices_[i].t;
}

float2 Vertex_stream_interleaved::uv(uint32_t i) const noexcept {
    return vertices_[i].uv;
}

uint8_t Vertex_stream_interleaved::bitangent_sign(uint32_t i) const noexcept {
    return vertices_[i].bitangent_sign;
}

Vertex_stream_separate::Vertex_stream_separate(uint32_t num_vertices, packed_float3 const* p,
                                               packed_float3 const* n, packed_float3 const* t,
                                               float2 const* uv, uint8_t const* bts)
    : Vertex_stream(num_vertices), p_(p), n_(n), t_(t), uv_(uv), bts_(bts) {}

void Vertex_stream_separate::release() noexcept {
    delete[] p_;
    delete[] n_;
    delete[] t_;
    delete[] uv_;
    delete[] bts_;
}

packed_float3 Vertex_stream_separate::p(uint32_t i) const noexcept {
    return p_[i];
}

packed_float3 Vertex_stream_separate::n(uint32_t i) const noexcept {
    return n_[i];
}

packed_float3 Vertex_stream_separate::t(uint32_t i) const noexcept {
    return t_[i];
}

float2 Vertex_stream_separate::uv(uint32_t i) const noexcept {
    return uv_[i];
}

uint8_t Vertex_stream_separate::bitangent_sign(uint32_t i) const noexcept {
    return bts_[i];
}

}  // namespace scene::shape
