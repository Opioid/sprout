#include "triangle_mesh_exporter.hpp"

#include "base/math/vector3.inl"
#include "base/string/string.hpp"
#include "triangle_json_handler.hpp"
#include "vertex_encoding.inl"
#include "vertex_layout_description.hpp"

#include <fstream>
#include <sstream>

#include <iostream>
#include "base/math/print.hpp"

namespace scene::shape::triangle {

static void newline(std::ostream& stream, uint32_t num_tabs) noexcept {
    stream << std::endl;

    for (uint32_t i = 0; i < num_tabs; ++i) {
        stream << '\t';
    }
}

static void binary_tag(std::ostream& stream, uint64_t offset, uint64_t size) noexcept {
    stream << "\"binary\":{\"offset\":" << offset << ",\"size\":" << size << "}";
}

void Exporter::write(std::string const& filename, Json_handler& handler) noexcept {
    std::string const out_name = string::extract_filename(filename) + ".sub";

    std::cout << "Export " << out_name << std::endl;

    std::ofstream stream(out_name, std::ios::binary);

    if (!stream) {
        return;
    }

    const char header[] = "SUB\000";
    stream.write(header, sizeof(char) * 4);

    std::stringstream jstream;

    newline(jstream, 0);
    jstream << "{";

    newline(jstream, 1);
    jstream << "\"geometry\":{";

    newline(jstream, 2);
    jstream << "\"parts\":[";

    auto const& parts = handler.parts();

    for (size_t i = 0, len = parts.size(); i < len; ++i) {
        newline(jstream, 3);

        auto const& p = parts[i];
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

    static bool constexpr interleaved_vertex_stream = false;

    auto const& vertices = handler.vertices();

    uint64_t const num_vertices  = vertices.size();
    uint64_t const vertices_size = interleaved_vertex_stream
                                       ? num_vertices * sizeof(Vertex)
                                       : num_vertices * Vertex::unpadded_size();

    binary_tag(jstream, 0, vertices_size);
    jstream << ",";

    newline(jstream, 3);
    jstream << "\"num_vertices\":" << num_vertices << ",";

    newline(jstream, 3);
    jstream << "\"layout\":[";

    Vertex_layout_description::Element element;

    using Encoding = scene::shape::triangle::Vertex_layout_description::Encoding;

    if (interleaved_vertex_stream) {
        newline(jstream, 4);
        element.semantic_name = "Position";
        element.encoding      = Encoding::Float32x3;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Normal";
        element.byte_offset   = 12;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Tangent";
        element.byte_offset   = 24;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Texture_coordinate";
        element.encoding      = Encoding::Float32x2;
        element.byte_offset   = 36;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Bitangent_sign";
        element.encoding      = Encoding::UInt8;
        element.byte_offset   = 44;
        jstream << element;
    } else {
        newline(jstream, 4);
        element.semantic_name = "Position";
        element.encoding      = Encoding::Float32x3;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Normal";
        element.stream        = 1;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Tangent";
        element.stream        = 2;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Texture_coordinate";
        element.encoding      = Encoding::Float32x2;
        element.stream        = 3;
        jstream << element << ",";

        newline(jstream, 4);
        element.semantic_name = "Bitangent_sign";
        element.encoding      = Encoding::UInt8;
        element.stream        = 4;
        jstream << element;
    }

    // close layout
    newline(jstream, 3);
    jstream << "]";

    // close vertices
    newline(jstream, 2);
    jstream << "},";

    // indices
    newline(jstream, 2);
    jstream << "\"indices\":{";

    int64_t max_index_delta = 0;

    {
        int64_t previous_index = 0;
        for (auto const& t : handler.triangles()) {
            int64_t const a = int64_t(t.i[0]);

            int64_t delta_index = a - previous_index;
            max_index_delta     = std::max(delta_index, max_index_delta);

            int64_t const b = int64_t(t.i[1]);

            delta_index     = b - a;
            max_index_delta = std::max(delta_index, max_index_delta);

            int64_t const c = int64_t(t.i[2]);

            delta_index     = c - b;
            max_index_delta = std::max(delta_index, max_index_delta);

            previous_index = c;
        }
    }

    bool   delta_indices = false;
    size_t index_bytes   = 4;

    if (max_index_delta <= 0x000000000000FFFF) {
        if (max_index_delta <= 0x0000000000008000) {
            delta_indices = true;
        }

        index_bytes = 2;
    } else if (max_index_delta <= 0x0000000080000000) {
        delta_indices = true;
    }

    newline(jstream, 3);
    uint64_t const num_indices = handler.triangles().size() * 3;
    binary_tag(jstream, vertices_size, num_indices * index_bytes);
    jstream << ",";

    newline(jstream, 3);
    jstream << "\"num_indices\":" << num_indices << ",";

    newline(jstream, 3);
    jstream << "\"encoding\":";

    if (4 == index_bytes) {
        if (delta_indices) {
            jstream << "\"Int32\"";
        } else {
            jstream << "\"UInt32\"";
        }
    } else {
        if (delta_indices) {
            jstream << "\"Int16\"";
        } else {
            jstream << "\"UInt16\"";
        }
    }

    // close indices
    newline(jstream, 2);
    jstream << "}";

    // close geometry
    newline(jstream, 1);
    jstream << "}";

    // close start
    newline(jstream, 0);
    jstream << "}";

    newline(jstream, 0);

    std::string const json_string = jstream.str();
    uint64_t const    json_size   = json_string.size() - 1;
    stream.write(reinterpret_cast<char const*>(&json_size), sizeof(uint64_t));
    stream.write(reinterpret_cast<char const*>(json_string.data()), json_size * sizeof(char));

    // binary stuff

    if (!handler.has_texture_coordinates()) {
        // If no tangents were loaded, explicitely set them to zero.
        // This can potentially help with compression.
        for (auto& v : handler.vertices()) {
            v.uv = float2(0.f);
        }
    }

    if (interleaved_vertex_stream) {
        stream.write(reinterpret_cast<char const*>(vertices.data()), vertices_size);
    } else {
        for (uint32_t i = 0; i < num_vertices; ++i) {
            stream.write(reinterpret_cast<char const*>(&vertices[i].p), sizeof(packed_float3));
        }

        for (uint32_t i = 0; i < num_vertices; ++i) {
            stream.write(reinterpret_cast<char const*>(&vertices[i].n), sizeof(packed_float3));
        }

        for (uint32_t i = 0; i < num_vertices; ++i) {
            stream.write(reinterpret_cast<char const*>(&vertices[i].t), sizeof(packed_float3));
        }

        for (uint32_t i = 0; i < num_vertices; ++i) {
            stream.write(reinterpret_cast<char const*>(&vertices[i].uv), sizeof(float2));
        }

        for (uint32_t i = 0; i < num_vertices; ++i) {
            stream.write(reinterpret_cast<char const*>(&vertices[i].bitangent_sign),
                         sizeof(uint8_t));
        }
    }

    auto const& triangles = handler.triangles();

    if (4 == index_bytes) {
        int32_t previous_index = 0;
        for (auto const& t : triangles) {
            if (delta_indices) {
                int32_t const a = int32_t(t.i[0]);

                int32_t delta_index = a - previous_index;
                stream.write(reinterpret_cast<char const*>(&delta_index), sizeof(int32_t));

                int32_t const b = int32_t(t.i[1]);

                delta_index = b - a;
                stream.write(reinterpret_cast<char const*>(&delta_index), sizeof(int32_t));

                int32_t const c = int32_t(t.i[2]);

                delta_index = c - b;
                stream.write(reinterpret_cast<char const*>(&delta_index), sizeof(int32_t));

                previous_index = c;
            } else {
                stream.write(reinterpret_cast<char const*>(t.i), 3 * sizeof(uint32_t));
            }
        }
    } else {
        int32_t previous_index = 0;
        for (auto const& t : triangles) {
            if (delta_indices) {
                int32_t const a = int32_t(t.i[0]);

                int16_t delta_index = int16_t(a - previous_index);
                stream.write(reinterpret_cast<char const*>(&delta_index), sizeof(int16_t));

                int32_t const b = int32_t(t.i[1]);

                delta_index = int16_t(b - a);
                stream.write(reinterpret_cast<char const*>(&delta_index), sizeof(int16_t));

                int32_t const c = int32_t(t.i[2]);

                delta_index = int16_t(c - b);
                stream.write(reinterpret_cast<char const*>(&delta_index), sizeof(int16_t));

                previous_index = c;
            } else {
                uint16_t const a = static_cast<uint16_t>(t.i[0]);
                stream.write(reinterpret_cast<char const*>(&a), sizeof(uint16_t));

                uint16_t const b = static_cast<uint16_t>(t.i[1]);
                stream.write(reinterpret_cast<char const*>(&b), sizeof(uint16_t));

                uint16_t const c = static_cast<uint16_t>(t.i[2]);
                stream.write(reinterpret_cast<char const*>(&c), sizeof(uint16_t));
            }
        }
    }
}

}  // namespace scene::shape::triangle
