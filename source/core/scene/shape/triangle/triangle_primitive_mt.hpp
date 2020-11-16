#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_HPP

#include "base/math/vector4.hpp"
#include "scene/shape/shape_vertex.hpp"

namespace math {
struct ray;
}

namespace scene::shape::triangle {

struct Triangle_MT {
    Triangle_MT(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c,
                float bitangent_sign, uint32_t material_index);

    struct Vertex {
        Vertex(const shape::Vertex& v);

        float3 p, n, t;
        float2 uv;
    };

    Vertex   a, b, c;
    float    bitangent_sign;
    uint32_t material_index;

    bool intersect(ray& ray, float2& uv) const;

    bool intersect_p(ray const& ray) const;

    void interpolate(float2 uv, float3& p, float3& n, float2& tc) const;

    void interpolate(float2 uv, float3& p, float2& tc) const;

    void interpolate(float2 uv, float3& p) const;

    void interpolate_data(float2 uv, float3& n, float3& t, float2& tc) const;

    float2 interpolate_uv(float2 uv) const;

    float3 normal() const;

    float area() const;
    float area(float3_p scale) const;
};

struct Shading_vertex_MT {
    float3   n, t;
    float2   uv;
    float    bitangent_sign;
    uint32_t material_index;
};

struct Shading_vertex_MTC {
    Shading_vertex_MTC();

    Shading_vertex_MTC(float3_p n, float3_p t, float2 uv);

    float4 n_u;
    float4 t_v;
};

struct Vertex_MTC {
    Vertex_MTC(packed_float3_p p, packed_float3_p n, packed_float3_p t, float2 uv);

    float3 p;
    float4 n_u;
    float4 t_v;
};

}  // namespace scene::shape::triangle

#endif
