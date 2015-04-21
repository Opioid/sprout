#include "triangle_mesh_provider.hpp"
#include "triangle_mesh.hpp"
#include "triangle_primitive.hpp"
#include "bvh/triangle_bvh_builder.hpp"
#include "base/math/vector.inl"
#include "base/json/json.hpp"
#include <fstream>
#include <iostream>

namespace scene { namespace shape { namespace triangle {

std::shared_ptr<Mesh> Provider::load(const std::string& filename, uint32_t /*flags*/) {
	std::ifstream stream(filename, std::ios::binary);
	if (!stream) {
		return nullptr;
	}

	auto root = json::parse(stream);

	std::vector<Index_triangle> triangles;
	std::vector<Vertex> vertices;

	for (auto n = root->MemberBegin(); n != root->MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("geometry" == node_name) {
			load_mesh_data(node_value, triangles, vertices);
		}
	}

	auto mesh = std::make_shared<Mesh>();

	bvh::Builder builder;
	builder.build(mesh->tree_, triangles, vertices, 8);

	mesh->aabb_ = mesh->tree_.aabb();

	return mesh;
}

void Provider::load_mesh_data(const rapidjson::Value& geometry_value, std::vector<Index_triangle>& triangles, std::vector<Vertex>& vertices) const {
	std::string primitive_topology = json::read_string(geometry_value, "primitive_topology");
	if ("triangle_list" != primitive_topology) {
		return;
	}

	// checking for positions now, but handling them later
	const rapidjson::Value::ConstMemberIterator positions_node = geometry_value.FindMember("positions");
	if (geometry_value.MemberEnd() == positions_node) {
		return;
	}

	// parts
	const rapidjson::Value::ConstMemberIterator groups_node = geometry_value.FindMember("groups");
	if (geometry_value.MemberEnd() == groups_node) {
		return;
	}

	std::vector<Group> groups;
	load_groups(groups_node->value, groups);

	// indices
	const rapidjson::Value::ConstMemberIterator indices_node = geometry_value.FindMember("indices");
	if (geometry_value.MemberEnd() == indices_node) {
		return;
	}

	const rapidjson::Value& indices_value = indices_node->value;

	for (auto& p : groups) {
		uint32_t triangles_start = p.start_index / 3;
		uint32_t triangles_end = (p.start_index + p.num_indices) / 3;

		for (uint32_t i = triangles_start; i < triangles_end; ++i) {
			uint32_t a = (indices_value)[i * 3 + 0].GetUint();
			uint32_t b = (indices_value)[i * 3 + 1].GetUint();
			uint32_t c = (indices_value)[i * 3 + 2].GetUint();

			triangles.push_back(Index_triangle{a, b, c, p.material_index});
		}
	}

	// vertices
	const rapidjson::Value& positions_value = positions_node->value;
	size_t num_vertices = positions_value.Size();

	vertices.resize(num_vertices);

	size_t i = 0;
	for (auto p = positions_value.Begin(); p != positions_value.End(); ++p, ++i) {
		vertices[i].p = json::read_float3(*p);
	}

	bool has_normals = false;
	bool has_tangents = false;

	for (auto n = geometry_value.MemberBegin(); n != geometry_value.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const rapidjson::Value& node_value = n->value;

		if ("normals" == node_name) {
			size_t i = 0;
			for (auto n = node_value.Begin(); n != node_value.End(); ++n, ++i) {
				vertices[i].n = json::read_float3(*n);
			}

			has_normals = true;
		} else if ("tangents_and_bitangent_signs" == node_name) {
			size_t i = 0;
			for (auto n = node_value.Begin(); n != node_value.End(); ++n, ++i) {
				math::float4 t =  json::read_float4(*n);
				vertices[i].t = t.xyz;
				vertices[i].bitangent_sign = t.w;
			}

			has_tangents = true;
		} else if ("texture_coordinates_0" == node_name) {
			size_t i = 0;
			for (auto n = node_value.Begin(); n != node_value.End(); ++n, ++i) {
				vertices[i].uv = json::read_float2(*n);
			}
		}
	}

	if (has_normals && !has_tangents) {
		// If normals but no tangents were loaded, compute the tangent space manually

		for (auto& v : vertices) {
			math::float3 t;
			math::float3 b;

			math::coordinate_system(v.n, t, b);

			v.t = t;
		}
	}
}

void Provider::load_groups(const rapidjson::Value& groups_value, std::vector<Group>& groups) const {
	if (!groups_value.IsArray()) {
		return;
	}

	for (auto g = groups_value.Begin(); g != groups_value.End(); ++g) {
		groups.push_back(Group{
			json::read_uint(*g, "material_index"),
			json::read_uint(*g, "start_index"),
			json::read_uint(*g, "num_indices"),
		});
	}
}

}}}

