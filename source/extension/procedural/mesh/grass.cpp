#include "grass.hpp"
#include "core/scene/shape/geometry/vertex.hpp"
#include "core/scene/shape/triangle/triangle_primitive.hpp"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/random/generator.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/math/sampling/sampling.inl"

namespace procedural { namespace mesh {

std::shared_ptr<scene::shape::Shape> Grass::create_mesh(const rapidjson::Value& mesh_value, thread::Pool& thread_pool) {

	std::vector<scene::shape::triangle::Index_triangle> triangles;
	std::vector<scene::shape::Vertex> vertices;
	uint32_t num_parts = 1;
	scene::shape::triangle::BVH_preset bvh_preset = scene::shape::triangle::BVH_preset::Slow;

	const uint32_t num_vertices = 15;

/*
	float r = 0.f;
	float l = 0.5f;
	add_blade(math::packed_float3(0.2f, 0.f, 0.f), r, l, 0, triangles, vertices);

	r = math::degrees_to_radians(90.f);
	add_blade(math::packed_float3(-0.2f, 0.f, 0.f), r, l, num_vertices, triangles, vertices);
*/

	math::random::Generator rng(0, 1, 2, 3);

	uint32_t num_blades = 192 * 1024;

	math::float2 start(-1.f, -1.f);
	math::float2 end  ( 1.f,  1.f);

	math::float2 range = end - start;

	for (uint32_t i = 0; i < num_blades; ++i) {
		math::float2 s = math::ems(i, 3, 4);

	//	math::float2 p = start + s * range;
		math::float2 p = 6.f * math::sample_disk_concentric(s);

		float rotation_y = rng.random_float() * 2.f * math::Pi;

		float h = 0.8 + 0.2 * rng.random_float();

		add_blade(math::packed_float3(p.x, 0.f, p.y), rotation_y, 0.25f, h, i * num_vertices, triangles, vertices);
	}

	calculate_normals(triangles, vertices);

	return scene::shape::triangle::Provider::create_mesh(triangles, vertices, num_parts, bvh_preset, thread_pool);
}

void Grass::add_blade(const math::packed_float3& offset,
					  float rotation_y, float lean_factor, float height,
					  uint32_t vertex_offset,
					  std::vector<scene::shape::triangle::Index_triangle>& triangles,
					  std::vector<scene::shape::Vertex>& vertices) const {

	scene::shape::triangle::Index_triangle tri;
	tri.material_index = 0;

	const uint32_t num_segments = 4;
	uint32_t vertices_per_segment = 3;

	for (uint32_t i = 0; i < num_segments; ++i) {
		tri.a = vertex_offset + 0;
		tri.b = vertex_offset + 3;
		tri.c = vertex_offset + 1;
		triangles.push_back(tri);

		tri.a = vertex_offset + 3;
		tri.b = vertex_offset + 4;
		tri.c = vertex_offset + 1;
		triangles.push_back(tri);

		tri.a = vertex_offset + 1;
		tri.b = vertex_offset + 4;
		tri.c = vertex_offset + 2;
		triangles.push_back(tri);

		tri.a = vertex_offset + 4;
		tri.b = vertex_offset + 5;
		tri.c = vertex_offset + 2;
		triangles.push_back(tri);

		vertex_offset += vertices_per_segment;
	}

	scene::shape::Vertex v;
	v.t = math::packed_float3(1.f, 0.f, 0.f);
	v.bitangent_sign = 1.f;

	math::float3x3 rotation = math::float3x3::identity;
	math::set_rotation_y(rotation, rotation_y);

	math::packed_float3 p;

	float segment_leans[num_segments + 1];
	segment_leans[0] = 0.f;
	segment_leans[1] = 0.1f * lean_factor;
	segment_leans[2] = 0.25f * lean_factor;
	segment_leans[3] = 0.4f * lean_factor;
	segment_leans[4] = 0.45f * lean_factor;



	math::packed_float3 segments[num_segments + 1];
	segments[0] = math::packed_float3(0.025f,  0.f,                                          0.008f);
	segments[1] = math::packed_float3(0.0225f, segments[0].y + 0.35f * height - segment_leans[1] * segment_leans[1],  0.007f);
	segments[2] = math::packed_float3(0.0175f, segments[1].y + 0.3f  * height - segment_leans[2] * segment_leans[2],  0.006f);
	segments[3] = math::packed_float3(0.0125f, segments[2].y + 0.2f  * height - segment_leans[3] * segment_leans[3],  0.005f);
	segments[4] = math::packed_float3(0.005f,  segments[3].y + 0.075 * height - segment_leans[4] * segment_leans[4], 0.004f);

	math::packed_float3 scale(0.2f, 0.2f, 0.2f);
	for (uint32_t i = 0, len = num_segments + 1; i < len; ++i) {
		segments[i] *= scale;
	}

	for (uint32_t i = 0, len = num_segments + 1; i < len; ++i) {
		p = math::packed_float3(-segments[i].x, segments[i].y, segments[i].z + segment_leans[i]) * rotation + offset;
		v.p = p;
		vertices.push_back(v);

		p = math::packed_float3(0.f, segments[i].y, -segments[i].z + segment_leans[i]) * rotation + offset;
		v.p = p;
		vertices.push_back(v);

		p = math::packed_float3(segments[i].x, segments[i].y, segments[i].z + segment_leans[i]) * rotation + offset;
		v.p = p;
		vertices.push_back(v);
	}
}

void Grass::calculate_normals(std::vector<scene::shape::triangle::Index_triangle>& triangles,
							  std::vector<scene::shape::Vertex>& vertices) {
	std::vector<math::packed_float3> triangle_normals(triangles.size());

	for (size_t i = 0, len = triangles.size(); i < len; ++i) {
		const auto& tri = triangles[i];

		const auto& a = vertices[tri.a].p;
		const auto& b = vertices[tri.b].p;
		const auto& c = vertices[tri.c].p;

		auto e1 = b - a;
		auto e2 = c - a;

		triangle_normals[i] = math::normalized(math::cross(e1, e2));
	}

	struct Shading_normal {
		Shading_normal() : sum(0.f, 0.f, 0.f), num(0) {}

		math::packed_float3 sum;
		uint32_t num;
	};

	std::vector<Shading_normal> normals(vertices.size());

	for (size_t i = 0, len = triangles.size(); i < len; ++i) {
		const auto& tri = triangles[i];

		normals[tri.a].sum += triangle_normals[i];
		++normals[tri.a].num;

		normals[tri.b].sum += triangle_normals[i];
		++normals[tri.b].num;

		normals[tri.c].sum += triangle_normals[i];
		++normals[tri.c].num;
	}

	for (size_t i = 0, len = vertices.size(); i < len; ++i) {
		vertices[i].n = math::normalized(normals[i].sum / static_cast<float>(normals[i].num));
	}
}

}}
