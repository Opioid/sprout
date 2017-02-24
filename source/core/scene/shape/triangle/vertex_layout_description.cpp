#include "vertex_layout_description.hpp"
#include <sstream>

namespace scene { namespace shape { namespace triangle {



}}}

using Encoding = scene::shape::triangle::Vertex_layout_description::Encoding;

// Why does this not work?
std::stringstream& operator<<(std::stringstream& stream, Encoding encoding) {
	switch (encoding) {
	case Encoding::Float32:
		stream << "float32";
		break;
	case Encoding::Float32x2:
		stream << "float32x2";
		break;
	case Encoding::Float32x3:
		stream << "float32x3";
		break;
	default:
		stream << "unknown";
	}

	return stream;
}

void print(std::stringstream& stream, Encoding encoding) {
	switch (encoding) {
	case Encoding::Float32:
		stream << "Float32";
		break;
	case Encoding::Float32x2:
		stream << "Float32x2";
		break;
	case Encoding::Float32x3:
		stream << "Float32x3";
		break;
	default:
		stream << "Unknown";
	}
}

using Element = scene::shape::triangle::Vertex_layout_description::Element;

std::stringstream& operator<<(std::stringstream& stream, const Element& element) {
	stream << "{";
	stream << "\"semantic_name\":\"" << element.semantic_name << "\",";
	stream << "\"semantic_index\":" << element.semantic_index << ",";
//	stream << "\"encoding\":\"" << element.encoding << "\"";

	stream << "\"encoding\":\""; print(stream, element.encoding); stream << "\",";
	stream << "\"stream\":" << element.stream << ",";
	stream << "\"byte_offset\":" << element.byte_offset;

	stream << "}";

	return stream;
}
