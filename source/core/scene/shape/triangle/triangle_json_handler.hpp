#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include <vector>
#include <cstdint>

namespace scene { namespace shape { namespace triangle {

class Json_handler {
public:

	struct Group {
		uint32_t material_index;
		uint32_t start_index;
		uint32_t num_indices;
	};

	Json_handler();

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

	const std::vector<Group>& groups() const;

	const std::vector<uint32_t>& indices() const;

	const std::vector<Vertex>& vertices() const;
	std::vector<Vertex>& vertices();

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

	enum class Object {
		Unknown,
		Group
	};

	std::vector<Group> groups_;

	std::vector<uint32_t> indices_;

	std::vector<Vertex> vertices_;

	Number expected_number_;
	Object expected_object_;

	size_t current_vertex_;
	size_t current_array_index_;

	bool has_positions_;
	bool has_normals_;
	bool has_tangents_;
};

}}}
