#ifndef SU_CORE_SCENE_SHAPE_VERTEX_HPP
#define SU_CORE_SCENE_SHAPE_VERTEX_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Vertex {
    packed_float3 p;
    packed_float3 n;
    packed_float3 t;

    float2 uv;

    uint8_t bitangent_sign;

    uint8_t pad[3];

    static size_t unpadded_size() noexcept;
};

class Vertex_stream {
  public:
    Vertex_stream(uint32_t num_vertices) noexcept;

    uint32_t num_vertices() const noexcept;

    virtual ~Vertex_stream() noexcept;

    virtual void release() noexcept = 0;

    virtual float const* p(uint32_t i) const noexcept = 0;

    virtual float const* n(uint32_t i) const noexcept = 0;

    virtual float const* t(uint32_t i) const noexcept = 0;

    virtual float2 uv(uint32_t i) const noexcept = 0;

    virtual uint8_t bitangent_sign(uint32_t i) const noexcept = 0;

  private:
    uint32_t num_vertices_;
};

class Vertex_stream_interleaved final : public Vertex_stream {
  public:
    Vertex_stream_interleaved(uint32_t num_vertices, Vertex const* vertices);

    void release() noexcept override final;

    float const* p(uint32_t i) const noexcept override final;

    float const* n(uint32_t i) const noexcept override final;

    float const* t(uint32_t i) const noexcept override final;

    float2 uv(uint32_t i) const noexcept override final;

    uint8_t bitangent_sign(uint32_t i) const noexcept override final;

  private:
    Vertex const* vertices_;
};

class Vertex_stream_separate final : public Vertex_stream {
  public:
    Vertex_stream_separate(uint32_t num_vertices, packed_float3 const* p, packed_float3 const* n,
                           packed_float3 const* t, float2 const* uv, uint8_t const* bts);

    void release() noexcept override final;

    float const* p(uint32_t i) const noexcept override final;

    float const* n(uint32_t i) const noexcept override final;

    float const* t(uint32_t i) const noexcept override final;

    float2 uv(uint32_t i) const noexcept override final;

    uint8_t bitangent_sign(uint32_t i) const noexcept override final;

  private:
    packed_float3 const* p_;
    packed_float3 const* n_;
    packed_float3 const* t_;
    float2 const*        uv_;
    uint8_t const*       bts_;
};

}  // namespace scene::shape

#endif
