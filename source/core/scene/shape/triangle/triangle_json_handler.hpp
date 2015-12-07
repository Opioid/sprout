#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include <vector>
#include <string>
#include <cstdint>

namespace scene { namespace shape { namespace triangle {

class Json_handler {
public:

	struct Part {
		uint32_t material_index;
		uint32_t start_index;
		uint32_t num_indices;
	};

	Json_handler();

	void clear();
	void create_part();

	bool Null();
	bool Bool(bool b);
	bool Int(int i);
	bool Uint(unsigned i);
	bool Int64(int64_t i);
	bool Uint64(uint64_t i);
	bool Double(double d);
	bool String(const char* str, size_t length, bool copy);
	bool StartObject();
	bool Key(const char* str, size_t length, bool copy);
	bool EndObject(size_t memberCount);
	bool StartArray();
	bool EndArray(size_t elementCount);

	bool has_positions() const;
	bool has_normals() const;
	bool has_tangents() const;
	bool has_texture_coordinates() const;

	const std::vector<Part>& parts() const;

	const std::vector<uint32_t>& indices() const;

	const std::vector<Vertex>& vertices() const;
	std::vector<Vertex>& vertices();

	const std::vector<std::string>& morph_targets() const;

private:

	void handle_vertex(float v);

	void add_position(float v);
	void add_normal(float v);
	void add_tangent(float v);
	void add_texture_coordinate(float v);

	enum class Number {
		Unknown,
		Material_index,
		Start_index,
		Num_indices,
		Index,
		Position,
		Texture_coordinate_0,
		Normal,
		Tangent
	};

	enum class String_type {
		Unknown,
		Morph_target
	};

	enum class Object {
		Unknown,
		Geometry,
		Morph_targets,
		Part,
		Vertices
	};

	uint32_t object_level_;
	Object top_object_;

	std::vector<Part> parts_;

	std::vector<uint32_t> indices_;

	std::vector<Vertex> vertices_;

	Number expected_number_;
	String_type expected_string_;
	Object expected_object_;

	uint32_t current_vertex_;
	uint32_t current_vertex_component_;

	bool has_positions_;
	bool has_normals_;
	bool has_tangents_;
	bool has_texture_coordinates_;

	std::vector<std::string> morph_targets_;
};

}}}
