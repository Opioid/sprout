#include "grass.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/sample_distribution.inl"
#include "base/math/sampling.inl"
#include "base/math/vector3.inl"
#include "base/memory/variant_map.hpp"
#include "base/random/generator.inl"
#include "core/image/texture/sampler/address_mode.hpp"
#include "core/image/texture/sampler/sampler_linear_2d.inl"
#include "core/image/texture/texture_provider.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/scene/shape/shape_vertex.hpp"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "core/scene/shape/triangle/triangle_primitive.hpp"

namespace procedural::mesh {

std::shared_ptr<scene::shape::Shape> Grass::create_mesh(json::Value const& /*mesh_value*/,
                                                        resource::Manager& manager) {
    std::vector<scene::shape::triangle::Index_triangle> triangles;
    std::vector<scene::shape::Vertex>                   vertices;
    uint32_t                                            num_parts = 1;

    uint32_t const num_vertices = 16;

    /*
            float r = 0.f;
            float l = 0.f;
            float w = 1.5f;
            float h = 1.f;
            add_blade(packed_float3(0.2f, 0.f, 0.f), r, l, w, h, 0, triangles, vertices);

            r = math::degrees_to_radians(90.f);

            for (uint32_t i = 0; i < 10; ++i) {
                    l = 0.1f * static_cast<float>(i);
                    add_blade(packed_float3(-0.1f, 0.f, 0.f),
                                      r, l, w, h, i * num_vertices, triangles, vertices);
            }
    */

    //	memory::Variant_map options;
    //	options.insert("usage", image::texture::Provider::Usage::Mask);
    //	auto mask = manager.load<image::texture::Texture>("textures/how.png", options);;
    //	image::texture::sampler::Linear_2D<image::texture::sampler::Address_mode_repeat> sampler;

    rnd::Generator rng(0, 0);

    uint32_t num_blades = 160 * 1024;

    //	float2 start(-1.f,  1.f);
    //	float2 end  ( 1.f, -1.f);

    //	float2 range = end - start;

    for (uint32_t i = 0; i < num_blades; ++i) {
        float2 s = math::ems(i, 3, 4);

        //	float2 p = start + s * range;
        float2 p = 4.f * math::sample_disk_concentric(s);

        float randomness = 1.f;  // sampler.sample_1(*mask, s);

        float rotation_y = rng.random_float() * 2.f * Pi;

        rotation_y = math::lerp(0.25f * Pi, rotation_y, randomness);

        float l = 0.05f + 0.1f * rng.random_float();  //- (0.15f * randomness);
        float w = 0.1f + 0.05f * rng.random_float();
        float h = 0.2f + 0.15f * rng.random_float();  // + 0.15f - (0.15f * randomness);

        add_blade(float3(p[0], 0.f, p[1]), rotation_y, l, w, h, i * num_vertices, triangles,
                  vertices);
    }

    calculate_normals(triangles, vertices);

    return scene::shape::triangle::Provider::create_mesh(triangles, vertices, num_parts,
                                                         manager.thread_pool());
}

void Grass::add_blade(float3 const& offset, float rotation_y, float lean_factor, float width,
                      float height, uint32_t vertex_offset,
                      std::vector<scene::shape::triangle::Index_triangle>& triangles,
                      std::vector<scene::shape::Vertex>&                   vertices) const {
    scene::shape::triangle::Index_triangle tri;
    tri.material_index = 0;

    constexpr uint32_t num_segments         = 4;
    uint32_t           vertices_per_segment = 3;

    for (uint32_t i = 0; i < num_segments; ++i) {
        tri.i[0] = vertex_offset + 0;
        tri.i[1] = vertex_offset + 3;
        tri.i[2] = vertex_offset + 1;
        triangles.push_back(tri);

        tri.i[0] = vertex_offset + 3;
        tri.i[1] = vertex_offset + 4;
        tri.i[2] = vertex_offset + 1;
        triangles.push_back(tri);

        tri.i[0] = vertex_offset + 1;
        tri.i[1] = vertex_offset + 4;
        tri.i[2] = vertex_offset + 2;
        triangles.push_back(tri);

        tri.i[0] = vertex_offset + 4;
        tri.i[1] = vertex_offset + 5;
        tri.i[2] = vertex_offset + 2;
        triangles.push_back(tri);

        vertex_offset += vertices_per_segment;
    }

    tri.i[0] = vertex_offset + 0;
    tri.i[1] = vertex_offset + 3;
    tri.i[2] = vertex_offset + 1;
    triangles.push_back(tri);

    tri.i[0] = vertex_offset + 3;
    tri.i[1] = vertex_offset + 2;
    tri.i[2] = vertex_offset + 1;
    triangles.push_back(tri);

    scene::shape::Vertex v;
    v.t              = packed_float3(1.f, 0.f, 0.f);
    v.bitangent_sign = 1.f;

    float3x3 rotation;
    math::set_rotation_y(rotation, rotation_y);

    float max_width = 0.035f;

    float3 segment_controls[num_segments + 2];
    segment_controls[0] = float3(width * max_width, 0.f, width * 0.01f);
    segment_controls[1] = float3(width * -0.006f, height * 0.46f, width * -0.001f);
    segment_controls[2] = float3(width * -0.005f, height * 0.28f, width * -0.0015f);
    segment_controls[3] = float3(width * -0.004f, height * 0.13f, width * -0.002f);
    segment_controls[4] = float3(width * -0.006f, height * 0.08f, width * -0.001f);
    segment_controls[5] = float3(0.f, height * 0.05f, 0.f);

    float2 segment_uvs[num_segments + 2];

    float2 temp(0.f, 0.f);
    for (uint32_t i = 0, len = num_segments + 2; i < len; ++i) {
        temp += segment_controls[i].xy();

        segment_uvs[i] = float2(temp[0] / (width * max_width), 1.f - temp[1]);
    }

    struct Segment {
        float3 a, b;
    };

    Segment segments[num_segments + 2];

    float ax = -0.4f * Pi;

    float3x3 rx;
    math::set_rotation_x(rx, lean_factor * ax);
    segments[0].a = transform_vector(rx, segment_controls[0]);
    segments[0].b = transform_vector(rx,
                                     float3(0.f, segment_controls[0][1], -segment_controls[0][2]));

    ax += -0.1f;
    math::set_rotation_x(rx, lean_factor * ax);
    segments[1].a = segments[0].a + transform_vector(rx, segment_controls[1]);
    segments[1].b = segments[0].b + transform_vector(rx, float3(0.f, segment_controls[1][1],
                                                                -segment_controls[1][2]));

    ax += -0.5f;
    math::set_rotation_x(rx, lean_factor * ax);
    segments[2].a = segments[1].a + transform_vector(rx, segment_controls[2]);
    segments[2].b = segments[1].b + transform_vector(rx, float3(0.f, segment_controls[2][1],
                                                                -segment_controls[2][2]));

    ax += -0.6f;
    math::set_rotation_x(rx, lean_factor * ax);
    segments[3].a = segments[2].a + transform_vector(rx, segment_controls[3]);
    segments[3].b = segments[2].b + transform_vector(rx, float3(0.f, segment_controls[3][1],
                                                                -segment_controls[3][2]));

    ax += -0.8f;
    math::set_rotation_x(rx, lean_factor * ax);
    segments[4].a = segments[3].a + transform_vector(rx, segment_controls[4]);
    segments[4].b = segments[3].b + transform_vector(rx, float3(0.f, segment_controls[4][1],
                                                                -segment_controls[4][2]));

    ax += -0.4f;
    math::set_rotation_x(rx, lean_factor * ax);
    segments[5].a = segments[4].a + transform_vector(rx, segment_controls[5]);

    for (uint32_t i = 0, len = num_segments + 1; i < len; ++i) {
        v.p = packed_float3(transform_vector(rotation, float3(-segments[i].a[0], segments[i].a[1],
                                                              segments[i].a[2])) +
                            offset);

        v.uv = float2(1.f - segment_uvs[i][0], segment_uvs[i][1]);
        vertices.push_back(v);

        v.p = packed_float3(
            transform_vector(rotation, float3(0.f, segments[i].b[1], segments[i].b[2])) + offset);
        v.uv = float2(0.5f, segment_uvs[i][1]);
        vertices.push_back(v);

        v.p  = packed_float3(transform_vector(rotation, float3(segments[i].a[0], segments[i].a[1],
                                                              segments[i].a[2])) +
                            offset);
        v.uv = segment_uvs[i];
        vertices.push_back(v);
    }

    uint32_t i = num_segments + 1;

    v.p = packed_float3(
        transform_vector(rotation, float3(0.f, segments[i].a[1], segments[i].a[2])) + offset);
    v.uv = float2(0.5f, segment_uvs[i][1]);

    vertices.push_back(v);
}

void Grass::calculate_normals(std::vector<scene::shape::triangle::Index_triangle>& triangles,
                              std::vector<scene::shape::Vertex>&                   vertices) {
    std::vector<packed_float3> triangle_normals(triangles.size());

    for (size_t i = 0, len = triangles.size(); i < len; ++i) {
        auto const& tri = triangles[i];

        auto const& a = vertices[tri.i[0]].p;
        auto const& b = vertices[tri.i[1]].p;
        auto const& c = vertices[tri.i[2]].p;

        auto const e1 = b - a;
        auto const e2 = c - a;

        triangle_normals[i] = normalize(cross(e1, e2));
    }

    struct Shading_normal {
        Shading_normal() : sum(0.f, 0.f, 0.f), num(0) {}

        packed_float3 sum;
        uint32_t      num;
    };

    std::vector<Shading_normal> normals(vertices.size());

    for (size_t i = 0, len = triangles.size(); i < len; ++i) {
        auto const& tri = triangles[i];

        normals[tri.i[0]].sum += triangle_normals[i];
        ++normals[tri.i[0]].num;

        normals[tri.i[1]].sum += triangle_normals[i];
        ++normals[tri.i[1]].num;

        normals[tri.i[2]].sum += triangle_normals[i];
        ++normals[tri.i[2]].num;
    }

    for (size_t i = 0, len = vertices.size(); i < len; ++i) {
        vertices[i].n = normalize(normals[i].sum / static_cast<float>(normals[i].num));
    }
}

}  // namespace procedural::mesh
