#include "grass.hpp"
#include "core/image/texture/texture_provider.hpp"
#include "core/image/texture/sampler/address_mode.hpp"
#include "core/image/texture/sampler/sampler_linear_2d.inl"
#include "core/resource/resource_manager.hpp"
#include "core/scene/shape/shape_vertex.hpp"
#include "core/scene/shape/triangle/triangle_primitive.hpp"
#include "core/scene/shape/triangle/triangle_mesh_provider.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/sampling/sample_distribution.inl"
#include "base/math/sampling/sampling.inl"
#include "base/memory/variant_map.hpp"
#include "base/random/generator.inl"

namespace procedural { namespace mesh {

std::shared_ptr<scene::shape::Shape> Grass::create_mesh(const json::Value& /*mesh_value*/,
														resource::Manager& manager) {

	std::vector<scene::shape::triangle::Index_triangle> triangles;
	std::vector<scene::shape::Vertex> vertices;
	uint32_t num_parts = 1;
	scene::shape::triangle::BVH_preset bvh_preset = scene::shape::triangle::BVH_preset::Slow;

	const uint32_t num_vertices = 16;

/*
	float r = 0.f;
	float l = 0.f;
	float w = 1.5f;
	float h = 1.f;
	add_blade(math::packed_float3(0.2f, 0.f, 0.f), r, l, w, h, 0, triangles, vertices);

	r = math::degrees_to_radians(90.f);

	for (uint32_t i = 0; i < 10; ++i) {
		l = 0.1f * static_cast<float>(i);
		add_blade(math::packed_float3(-0.1f, 0.f, 0.f),
				  r, l, w, h, i * num_vertices, triangles, vertices);
	}
*/


//	memory::Variant_map options;
//	options.insert("usage", image::texture::Provider::Usage::Mask);
//	auto mask = manager.load<image::texture::Texture>("textures/how.png", options);;
//	image::texture::sampler::Linear_2D<image::texture::sampler::Address_mode_repeat> sampler;

	rnd::Generator rng(0, 1, 2, 3);

	uint32_t num_blades = 160 * 1024;

	float2 start(-1.f,  1.f);
	float2 end  ( 1.f, -1.f);

//	float2 range = end - start;

	for (uint32_t i = 0; i < num_blades; ++i) {
		float2 s = math::ems(i, 3, 4);

	//	float2 p = start + s * range;
		float2 p = 4.f * math::sample_disk_concentric(s);

		float randomness = 1.f;//sampler.sample_1(*mask, s);

		float rotation_y = rng.random_float() * 2.f * math::Pi;

		rotation_y = math::lerp(0.25f * math::Pi, rotation_y, randomness);

		float l = 0.05f + 0.1f  * rng.random_float();//- (0.15f * randomness);
		float w = 0.1f  + 0.05f * rng.random_float();
		float h = 0.2f  + 0.15f * rng.random_float();// + 0.15f - (0.15f * randomness);

		add_blade(float3(p.x, 0.f, p.y),
				  rotation_y, l, w, h, i * num_vertices, triangles, vertices);
	}

	calculate_normals(triangles, vertices);

	return scene::shape::triangle::Provider::create_mesh(triangles, vertices,
														 num_parts, bvh_preset,
														 manager.thread_pool());
}

void Grass::add_blade(float3_p offset,
					  float rotation_y, float lean_factor, float width, float height,
					  uint32_t vertex_offset,
					  std::vector<scene::shape::triangle::Index_triangle>& triangles,
					  std::vector<scene::shape::Vertex>& vertices) const {

	scene::shape::triangle::Index_triangle tri;
	tri.material_index = 0;

	const uint32_t num_segments = 4;
	uint32_t vertices_per_segment = 3;

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
	v.t = math::packed_float3(1.f, 0.f, 0.f);
	v.bitangent_sign = 1.f;

	math::float3x3 rotation;
	math::set_rotation_y(rotation, rotation_y);

	float max_width = 0.035f;

	float3 segment_controls[num_segments + 2];
	segment_controls[0] = float3(width *  max_width, 0.f,			  width *  0.01f);
	segment_controls[1] = float3(width * -0.006f,	  height * 0.46f, width * -0.001f);
	segment_controls[2] = float3(width * -0.005f,	  height * 0.28f, width * -0.0015f);
	segment_controls[3] = float3(width * -0.004f,	  height * 0.13f, width * -0.002f);
	segment_controls[4] = float3(width * -0.006f,	  height * 0.08f, width * -0.001f);
	segment_controls[5] = float3(0.f,				  height * 0.05f,  0.f);

	float2 segment_uvs[num_segments + 2];

	float2 temp(0.f, 0.f);
	for (uint32_t i = 0, len = num_segments + 2; i < len; ++i) {
		temp += segment_controls[i].xy;

		segment_uvs[i] = float2(temp.x / (width * max_width), 1.f - temp.y);
	}

	struct Segment {
		float3 a, b;
	};

	Segment segments[num_segments + 2];

	float3x3 rx;
	float ax = -0.4f * math::Pi;
	math::set_rotation_x(rx, lean_factor * ax);
	segments[0].a = segment_controls[0] * rx;
	segments[0].b = float3(0.f, segment_controls[0].y, -segment_controls[0].z) * rx;

	ax += -0.1f;
	math::set_rotation_x(rx, lean_factor * ax);
	segments[1].a = segments[0].a + segment_controls[1] * rx;
	segments[1].b = segments[0].b + float3(0.f,
										   segment_controls[1].y,
										   -segment_controls[1].z) * rx;

	ax += -0.5f;
	math::set_rotation_x(rx, lean_factor * ax);
	segments[2].a = segments[1].a + segment_controls[2] * rx;
	segments[2].b = segments[1].b + float3(0.f,
										   segment_controls[2].y,
										   -segment_controls[2].z) * rx;

	ax += -0.6f;
	math::set_rotation_x(rx, lean_factor * ax);
	segments[3].a = segments[2].a + segment_controls[3] * rx;
	segments[3].b = segments[2].b + float3(0.f,
										   segment_controls[3].y,
										   -segment_controls[3].z) * rx;

	ax += -0.8f;
	math::set_rotation_x(rx, lean_factor * ax);
	segments[4].a = segments[3].a + segment_controls[4] * rx;
	segments[4].b = segments[3].b + float3(0.f,
										   segment_controls[4].y,
										   -segment_controls[4].z) * rx;

	ax += -0.4f;
	math::set_rotation_x(rx, lean_factor * ax);
	segments[5].a = segments[4].a + segment_controls[5] * rx;

	for (uint32_t i = 0, len = num_segments + 1; i < len; ++i) {
		v.p = math::packed_float3(float3(-segments[i].a.x,
								   segments[i].a.y,
								   segments[i].a.z) * rotation + offset);

		v.uv = float2(1.f - segment_uvs[i].x, segment_uvs[i].y);
		vertices.push_back(v);

		v.p = math::packed_float3(float3(0.f, segments[i].b.y, segments[i].b.z) * rotation + offset);
		v.uv = float2(0.5f, segment_uvs[i].y);
		vertices.push_back(v);

		v.p = math::packed_float3(float3(segments[i].a.x,
								  segments[i].a.y,
								  segments[i].a.z) * rotation + offset);
		v.uv = segment_uvs[i];
		vertices.push_back(v);
	}

	uint32_t i = num_segments + 1;
	v.p = math::packed_float3(float3(0.f, segments[i].a.y, segments[i].a.z) * rotation + offset);
	v.uv = float2(0.5f, segment_uvs[i].y);
	vertices.push_back(v);
}

void Grass::calculate_normals(std::vector<scene::shape::triangle::Index_triangle>& triangles,
							  std::vector<scene::shape::Vertex>& vertices) {
	std::vector<math::packed_float3> triangle_normals(triangles.size());

	for (size_t i = 0, len = triangles.size(); i < len; ++i) {
		const auto& tri = triangles[i];

		const auto& a = vertices[tri.i[0]].p;
		const auto& b = vertices[tri.i[1]].p;
		const auto& c = vertices[tri.i[2]].p;

		const auto e1 = b - a;
		const auto e2 = c - a;

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

		normals[tri.i[0]].sum += triangle_normals[i];
		++normals[tri.i[0]].num;

		normals[tri.i[1]].sum += triangle_normals[i];
		++normals[tri.i[1]].num;

		normals[tri.i[2]].sum += triangle_normals[i];
		++normals[tri.i[2]].num;
	}

	for (size_t i = 0, len = vertices.size(); i < len; ++i) {
		vertices[i].n = math::normalized(normals[i].sum / static_cast<float>(normals[i].num));
	}
}

}}
