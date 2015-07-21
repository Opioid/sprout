#include "triangle_mesh_provider.hpp"
#include "resource/resource_provider.inl"
#include "triangle_mesh.hpp"
#include "triangle_primitive.hpp"
#include "bvh/triangle_bvh_builder.hpp"
#include "file/file_system.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"
#include "base/json/json.hpp"
#include "base/json/json_read_stream.hpp"
#include <iostream>

namespace scene { namespace shape { namespace triangle {

Provider::Provider(file::System& file_system) : resource::Provider<Mesh>(file_system) {}

std::shared_ptr<Mesh> Provider::load(const std::string& filename, uint32_t /*flags*/) {
	auto stream_pointer = file_system_.read_stream(filename);
	if (!*stream_pointer) {
		throw std::runtime_error("Stream could not be opened");
	}

	std::vector<Index_triangle> triangles;
	std::vector<Vertex> vertices;

	{
		json::Read_stream json_stream(*stream_pointer);

		Mesh_handler handler;

		rapidjson::Reader reader;

		reader.Parse(json_stream, handler);

		if (!handler.has_positions()) {
			throw std::runtime_error("Model does not contain vertex positions");
		}

		if (!handler.has_normals()) {
			// If normals were loaded assign identity
			// Might be smarter to throw an exception
			for (auto& v : handler.vertices()) {
				v.n = math::float3::identity;
			}
		}

		if (!handler.has_tangents()) {
			// If no tangents were loaded, compute the tangent space manually
			for (auto& v : handler.vertices()) {
				math::float3 b;
				math::coordinate_system(v.n, v.t, b);
			}
		}

		auto& indices = handler.indices();

		for (auto& p : handler.groups()) {
			uint32_t triangles_start = p.start_index / 3;
			uint32_t triangles_end = (p.start_index + p.num_indices) / 3;

			for (uint32_t i = triangles_start; i < triangles_end; ++i) {
				uint32_t a = indices[i * 3 + 0];
				uint32_t b = indices[i * 3 + 1];
				uint32_t c = indices[i * 3 + 2];

				triangles.push_back(Index_triangle{a, b, c, p.material_index});
			}
		}

		vertices.swap(handler.vertices());
	}

	auto mesh = std::make_shared<Mesh>();

	bvh::Builder builder;
	builder.build(mesh->tree_, triangles, vertices, 8);

	mesh->init();

	return mesh;

/*
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

	mesh->init();

	return mesh;
*/
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
//				vertices[i].bitangent_sign = t.w;
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
			math::float3 b;
			math::coordinate_system(v.n, v.t, b);
		}
	}
}

void Provider::load_groups(const rapidjson::Value& groups_value, std::vector<Group>& groups) const {
	if (!groups_value.IsArray()) {
		return;
	}

	uint32_t max_id = groups_value.Size() - 1;

	for (auto g = groups_value.Begin(); g != groups_value.End(); ++g) {
		groups.push_back(Group{
			std::min(json::read_uint(*g, "material_index"), max_id),
			json::read_uint(*g, "start_index"),
			json::read_uint(*g, "num_indices"),
		});
	}
}

Mesh_handler::Mesh_handler () :
	expected_number_(Number::Unknown),
	expected_object_(Object::Unknown),
	current_vertex_(0),
	current_array_index_(0),
	has_positions_(false),
	has_normals_(false),
	has_tangents_(false)
{}

bool Mesh_handler::Null() {
	return true;
}

bool Mesh_handler::Bool(bool /*b*/) {
	return true;
}

bool Mesh_handler::Int(int i) {
	handle_vertex(static_cast<float>(i));

	return true;
}

bool Mesh_handler::Uint(unsigned i) {
	switch (expected_number_) {
	case Number::Material_index:
		groups_.back().material_index = i;
		break;
	case Number::Start_index:
		groups_.back().start_index = i;
		break;
	case Number::Num_indices:
		groups_.back().num_indices = i;
		break;
	case Number::Index:
		indices_.push_back(i);
		break;
	default:
		handle_vertex(static_cast<float>(i));
		break;
	}

	return true;
}

bool Mesh_handler::Int64(int64_t /*i*/) {
	return true;
}

bool Mesh_handler::Uint64(uint64_t /*i*/) {
	return true;
}

bool Mesh_handler::Double(double d) {
	handle_vertex(static_cast<float>(d));

	return true;
}

bool Mesh_handler::String(const char* /*str*/, size_t /*length*/, bool /*copy*/) {
	return true;
}

bool Mesh_handler::StartObject() {
	switch (expected_object_) {
	case Object::Group:
		groups_.push_back(Group());
		break;
	default:
		break;
	}

	return true;
}

bool Mesh_handler::Key(const char* str, size_t /*length*/, bool /*copy*/) {
	std::string name(str);

	if ("groups" == name) {
		expected_object_ = Object::Group;
	} else if ("material_index" == name) {
		expected_number_ = Number::Material_index;
	} else if ("start_index" == name) {
		expected_number_ = Number::Start_index;
	} else if ("num_indices" == name) {
		expected_number_ = Number::Num_indices;
	} else if ("indices" == name) {
		expected_number_ = Number::Index;
	} else if ("positions" == name) {
		expected_number_ = Number::Position;
		current_vertex_ = 0;
		has_positions_ = true;
	} else if ("texture_coordinates_0" == name) {
		expected_number_ = Number::Texture_coordinate_0;
		current_vertex_ = 0;
	} else if ("normals" == name) {
		expected_number_ = Number::Normal;
		current_vertex_ = 0;
		has_normals_ = true;
	} else if ("tangents_and_bitangent_signs" == name) {
		expected_number_ = Number::Tangent;
		current_vertex_ = 0;
		has_tangents_ = true;
	}

	return true;
}

bool Mesh_handler::EndObject(size_t /*memberCount*/) {
	return true;
}

bool Mesh_handler::StartArray() {
	current_array_index_ = 0;
	return true;
}

bool Mesh_handler::EndArray(size_t /*elementCount*/) {
	// Of course not all arrays describe a vertex, so this value is only to be trusted while processing the array of vertices.
	// It works because current_vertex_ is set to 0 before the array of vertices starts
	++current_vertex_;
	return true;
}

bool Mesh_handler::has_positions() const {
	return has_positions_;
}

bool Mesh_handler::has_normals() const {
	return has_normals_;
}

bool Mesh_handler::has_tangents() const {
	return has_tangents_;
}

const std::vector<Mesh_handler::Group>& Mesh_handler::groups() const {
	return groups_;
}

const std::vector<uint32_t>& Mesh_handler::indices() const {
	return indices_;
}

const std::vector<Vertex>& Mesh_handler::vertices() const {
	return vertices_;
}

std::vector<Vertex>& Mesh_handler::vertices() {
	return vertices_;
}

void Mesh_handler::handle_vertex(float v) {
	switch (expected_number_) {
	case Number::Position:
		add_position(v);
		break;
	case Number::Normal:
		add_normal(v);
		break;
	case Number::Tangent:
		add_tangent(v);
		break;
	case Number::Texture_coordinate_0:
		add_texture_coordinate(v);
		break;
	default:
		break;
	}
}

void Mesh_handler::add_position(float v) {
	if (current_vertex_ == vertices_.size()) {
		vertices_.push_back(Vertex());
	}

	vertices_[current_vertex_].p.v[current_array_index_] = v;

	++current_array_index_;
}

void Mesh_handler::add_normal(float v) {
	if (current_vertex_ == vertices_.size()) {
		vertices_.push_back(Vertex());
	}

	vertices_[current_vertex_].n.v[current_array_index_] = v;

	++current_array_index_;
}

void Mesh_handler::add_tangent(float v) {
	if (current_vertex_ == vertices_.size()) {
		vertices_.push_back(Vertex());
	}

	if (current_array_index_ < 3) {
		vertices_[current_vertex_].t.v[current_array_index_] = v;
	}

	++current_array_index_;
}

void Mesh_handler::add_texture_coordinate(float v) {
	if (current_vertex_ == vertices_.size()) {
		vertices_.push_back(Vertex());
	}

	vertices_[current_vertex_].uv.v[current_array_index_] = v;

	++current_array_index_;
}

}}}

