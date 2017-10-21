#pragma once

#include "triangle_bvh_preset.hpp"
#include "triangle_primitive.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "rapidjson/reader.h"
#include <vector>
#include <string>
#include <cstdint>

namespace scene::shape::triangle {

struct Part {
	Part() = default;
	Part(uint32_t start_index, uint32_t num_indices, uint32_t material_index) :
		 start_index(start_index), num_indices(num_indices), material_index(material_index) {}

	uint32_t start_index;
	uint32_t num_indices;
	uint32_t material_index;
};

class Json_handler {

public:

	Json_handler();

	void clear(bool read_indices = true);
	void create_part();

	bool Null();
	bool Bool(bool b);
	bool Int(int i);
	bool Uint(unsigned i);
	bool Int64(int64_t i);
	bool Uint64(uint64_t i);
	bool Double(double d);
	bool RawNumber(const char* str, rapidjson::SizeType length, bool copy);
	bool String(const char* str, rapidjson::SizeType length, bool copy);
	bool StartObject();
	bool Key(const char* str, rapidjson::SizeType length, bool copy);
	bool EndObject(size_t memberCount);
	bool StartArray();
	bool EndArray(size_t elementCount);

	BVH_preset bvh_preset() const;

	bool has_positions() const;
	bool has_normals() const;
	bool has_tangents() const;
	bool has_texture_coordinates() const;

	const std::vector<Part>& parts() const;
	std::vector<Part>& parts();

	const std::vector<Index_triangle>& triangles() const;
	std::vector<Index_triangle>& triangles();

	const std::vector<Vertex>& vertices() const;
	std::vector<Vertex>& vertices();

	const std::vector<std::string>& morph_targets() const;

private:

	void add_index(uint32_t i);

	void increment_triangle_element();

	void handle_vertex(float v);

	void add_position(float v);
	void add_normal(float v);
	void add_tangent(float v);
	void add_texture_coordinate(float v);

	void increment_vertex_element(uint32_t num_elements);

	enum class Number {
		Undefined,
		Material_index,
		Start_index,
		Num_indices,
		Index,
		Position,
		Texture_coordinate_0,
		Normal,
		Tangent,
		Ignore
	};

	enum class String_type {
		Undefined,
		BVH_preset,
		Morph_target
	};

	enum class Object {
		Undefined,
		Geometry,
		Morph_targets,
		Part,
		Vertices
	};

	bool read_indices_;

	uint32_t object_level_;
	Object top_object_;

	std::vector<Part> parts_;

	std::vector<Index_triangle> triangles_;

	std::vector<Vertex> vertices_;

	Number expected_number_;
	String_type expected_string_;
	Object expected_object_;

	uint32_t current_triangle_;
	uint32_t current_triangle_element_;

	uint32_t current_vertex_;
	uint32_t current_vertex_element_;

	BVH_preset bvh_preset_;

	bool has_positions_;
	bool has_normals_;
	bool has_tangents_;
	bool has_texture_coordinates_;

	std::vector<std::string> morph_targets_;
};

}
