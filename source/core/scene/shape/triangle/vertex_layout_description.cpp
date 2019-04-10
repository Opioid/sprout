#include "vertex_layout_description.hpp"
#include <sstream>

namespace scene::shape::triangle {}

using Encoding = scene::shape::triangle::Vertex_layout_description::Encoding;

void print(std::stringstream& stream, Encoding encoding) {
    switch (encoding) {
        case Encoding::UInt8:
            stream << "UInt8";
            break;
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
            stream << "Undefined";
    }
}

using Element = scene::shape::triangle::Vertex_layout_description::Element;

std::stringstream& operator<<(std::stringstream& stream, Element const& element) {
    stream << "{";
    stream << "\"semantic_name\":\"" << element.semantic_name << "\",";
    stream << "\"semantic_index\":" << element.semantic_index << ",";
    //	stream << "\"encoding\":\"" << element.encoding << "\"";

    stream << "\"encoding\":\"";
    print(stream, element.encoding);
    stream << "\",";
    stream << "\"stream\":" << element.stream << ",";
    stream << "\"byte_offset\":" << element.byte_offset;

    stream << "}";

    return stream;
}
