#include "triangle_mesh_exporter.hpp"
#include "triangle_json_handler.hpp"
#include "vertex_layout_description.hpp"
#include "base/math/vector.inl"
#include <fstream>
#include <sstream>
#include <iostream>

namespace scene { namespace shape { namespace triangle {

std::string extract_filename(const std::string& filename) {
	size_t i = filename.find_last_of('/') + 1;
	return filename.substr(i, filename.find_first_of('.') - i);
}

void newline(std::ostream& stream, uint32_t num_tabs) {
	stream << std::endl;

	for (uint32_t i = 0; i < num_tabs; ++i) {
		stream << '\t';
	}
}

void binary_tag(std::ostream& stream, size_t offset, size_t size) {
	stream << "\"binary\":{\"offset\":" << offset <<",\"size\":" << size << "}";
}

void Exporter::write(const std::string& filename, const Json_handler& handler) {
	std::string out_name = extract_filename(filename) + ".sum";

	std::cout << "Export " << out_name << std::endl;

	std::ofstream stream(out_name, std::ios::binary);

	if (!stream) {
		return;
	}

	const char header[] = "SUM\005";
	stream.write(header, sizeof(char) * 4);

	std::stringstream jstream;

	newline(jstream, 0);
	jstream << "{";

	newline(jstream, 1);
	jstream << "\"geometry\":{";

	newline(jstream, 2);
	jstream << "\"parts\":[";

	const auto& parts = handler.parts();

	for (size_t i = 0, len = parts.size(); i < len; ++i) {
		newline(jstream, 3);

		const auto& p = parts[i];
		jstream << "{";
		jstream << "\"start_index\":" << p.start_index << ",";
		jstream << "\"num_indices\":" << p.num_indices << ",";
		jstream << "\"material_index\":" << p.material_index;
		jstream << "}";

		if (i < len - 1) {
			jstream << ",";
		}
	}


	// close parts
	newline(jstream, 2);
	jstream << "],";

	// vertices
	newline(jstream, 2);
	jstream << "\"vertices\":{";

	newline(jstream, 3);
	const auto& vertices = handler.vertices();
	size_t num_vertices = static_cast<uint32_t>(vertices.size());
	size_t vertices_size = num_vertices * sizeof(Vertex);
	binary_tag(jstream, 0, vertices_size);
	jstream << ",";

	newline(jstream, 3);
	jstream << "\"layout\":[";

	Vertex_layout_description::Element element;

	using Encoding = scene::shape::triangle::Vertex_layout_description::Encoding;

	newline(jstream, 4);
	element.semantic_name = "Position";
	element.encoding = Encoding::Float32x3;
	jstream << element << ",";

	newline(jstream, 4);
	element.semantic_name = "Normal";
	element.byte_offset = 12;
	jstream << element << ",";

	newline(jstream, 4);
	element.semantic_name = "Tangent";
	element.byte_offset = 24;
	jstream << element << ",";

	newline(jstream, 4);
	element.semantic_name = "Texture_coordinate";
	element.encoding = Encoding::Float32x2;
	element.byte_offset = 36;
	jstream << element << ",";

	newline(jstream, 4);
	element.semantic_name = "Bitangent_sign";
	element.encoding = Encoding::Float32;
	element.byte_offset = 44;
	jstream << element;

	// close layout
	newline(jstream, 3);
	jstream << "]";

	// close vertices
	newline(jstream, 2);
	jstream << "},";

	// indices
	newline(jstream, 2);
	jstream << "\"indices\":{";

	size_t index_bytes = 4;
	if (num_vertices <= 0x000000000000FFFF) {
		index_bytes = 2;
	}

	newline(jstream, 3);
	size_t num_indices = handler.triangles().size() * 3;
	binary_tag(jstream, vertices_size, num_indices * index_bytes);
	jstream << ",";

	newline(jstream, 3);
	jstream << "\"encoding\":";

	if (4 == index_bytes) {
		jstream << "\"UInt32\"";
	} else {
		jstream << "\"UInt16\"";
	}

	// close vertices
	newline(jstream, 2);
	jstream << "}";

	// close geometry
	newline(jstream, 1);
	jstream << "}";

	// close start
	newline(jstream, 0);
	jstream << "}";

	newline(jstream, 0);

	std::string json_string = jstream.str();
	uint64_t json_size = json_string.size() - 1;
	stream.write(reinterpret_cast<const char*>(&json_size), sizeof(uint64_t));
	stream.write(reinterpret_cast<const char*>(json_string.data()), json_size * sizeof(char));

	// binary stuff
	stream.write(reinterpret_cast<const char*>(vertices.data()), vertices_size);

	const auto& triangles = handler.triangles();

	if (4 == index_bytes) {
		for (const auto& t : triangles) {
			stream.write(reinterpret_cast<const char*>(&t.a), sizeof(uint32_t));
			stream.write(reinterpret_cast<const char*>(&t.b), sizeof(uint32_t));
			stream.write(reinterpret_cast<const char*>(&t.c), sizeof(uint32_t));
		}
	} else {
		for (const auto& t : triangles) {
			uint16_t a = static_cast<uint16_t>(t.a);
			stream.write(reinterpret_cast<const char*>(&a), sizeof(uint16_t));
			uint16_t b = static_cast<uint16_t>(t.b);
			stream.write(reinterpret_cast<const char*>(&b), sizeof(uint16_t));
			uint16_t c = static_cast<uint16_t>(t.c);
			stream.write(reinterpret_cast<const char*>(&c), sizeof(uint16_t));
		}
	}
}

}}}
