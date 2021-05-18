#ifndef SU_CORE_SCENE_SHAPE_VERTEX_HPP
#define SU_CORE_SCENE_SHAPE_VERTEX_HPP

#include "base/math/quaternion.hpp"
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

    static size_t unpadded_size();
};

class Vertex_stream {
  public:
    Vertex_stream(uint32_t num_vertices);

    uint32_t num_vertices() const;

    virtual ~Vertex_stream();

    virtual void release() = 0;

    virtual float3 p(uint32_t i) const = 0;

    virtual Quaternion frame(uint32_t i) const = 0;

    virtual float2 uv(uint32_t i) const = 0;

    virtual bool bitangent_sign(uint32_t i) const = 0;

  private:
    uint32_t num_vertices_;
};

class Vertex_stream_interleaved final : public Vertex_stream {
  public:
    Vertex_stream_interleaved(uint32_t num_vertices, Vertex const* vertices);

    void release() final;

    float3 p(uint32_t i) const final;

    Quaternion frame(uint32_t i) const final;

    float2 uv(uint32_t i) const final;

    bool bitangent_sign(uint32_t i) const final;

  private:
    Vertex const* vertices_;
};

class Vertex_stream_separate final : public Vertex_stream {
  public:
    Vertex_stream_separate(uint32_t num_vertices, packed_float3 const* p, packed_float3 const* n,
                           packed_float3 const* t, float2 const* uv, uint8_t const* bts);

    void release() final;

    float3 p(uint32_t i) const final;

    Quaternion frame(uint32_t i) const final;

    float2 uv(uint32_t i) const final;

    bool bitangent_sign(uint32_t i) const final;

  private:
    packed_float3 const* p_;
    packed_float3 const* n_;
    packed_float3 const* t_;
    float2 const*        uv_;
    uint8_t const*       bts_;
};

class Vertex_stream_separate_ts final : public Vertex_stream {
  public:
    Vertex_stream_separate_ts(uint32_t num_vertices, packed_float3 const* p, Quaternion const* ts,
                              float2 const* uv);

    void release() final;

    float3 p(uint32_t i) const final;

    Quaternion frame(uint32_t i) const final;

    float2 uv(uint32_t i) const final;

    bool bitangent_sign(uint32_t i) const final;

  private:
    packed_float3 const* p_;
    Quaternion const*    ts_;
    float2 const*        uv_;
};

class Vertex_stream_separate_compact final : public Vertex_stream {
  public:
    Vertex_stream_separate_compact(uint32_t num_vertices, packed_float3 const* p,
                                   packed_float3 const* n);

    void release() final;

    float3 p(uint32_t i) const final;

    Quaternion frame(uint32_t i) const final;

    float2 uv(uint32_t i) const final;

    bool bitangent_sign(uint32_t i) const final;

  private:
    packed_float3 const* p_;
    packed_float3 const* n_;
};

class Vertex_stream_CAPI final : public Vertex_stream {
  public:
    Vertex_stream_CAPI(uint32_t num_vertices, uint32_t positions_stride, uint32_t normals_stride,
                       uint32_t tangents_stride, uint32_t texture_coordinates_stride,
                       float const* positions, float const* normals, float const* tangents,
                       float const* texture_coordinates);

    void release() final;

    float3 p(uint32_t i) const final;

    Quaternion frame(uint32_t i) const final;

    float2 uv(uint32_t i) const final;

    bool bitangent_sign(uint32_t i) const final;

  private:
    uint32_t positions_stride_;
    uint32_t normals_stride_;
    uint32_t tangents_stride_;
    uint32_t texture_coordinates_stride_;

    float const* positions_;
    float const* normals_;
    float const* tangents_;
    float const* texture_coordinates_;
};

}  // namespace scene::shape

#endif
