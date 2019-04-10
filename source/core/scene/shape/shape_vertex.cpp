#include "shape_vertex.hpp"

namespace scene::shape {

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

}  // namespace scene::shape
