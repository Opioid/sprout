#include "triangle_mesh_exporter.hpp"
#include "base/math/vector3.inl"
#include "base/string/string.hpp"
#include "triangle_json_handler.hpp"
#include "vertex_layout_description.hpp"

#include <fstream>
#include <sstream>

#include <iostream>
#include "base/math/print.hpp"

namespace scene::shape::triangle::serialize {

static void newline(std::ostream& stream, uint32_t num_tabs) {
    stream << std::endl;

    for (uint32_t i = 0; i < num_tabs; ++i) {
        stream << '\t';
    }
}

static void binary_tag(std::ostream& stream, uint64_t offset, uint64_t size) {
    stream << R"("binary":{"offset":)" << offset << R"(,"size":)" << size << "}";
}

void Exporter::write(std::string const& filename, Json_handler& handler) {
    std::string const out_name = string::extract_filename(filename) + ".sub";

    std::cout << "Export " << out_name << std::endl;

    std::ofstream stream(out_name, std::ios::binary);

    if (!stream) {
        return;
    }

    const char header[] = "SUB\000";
    stream.write(header, sizeof(char) * 4);

    std::ostringstream jstream;

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

    int64_t max_index       = 0;
    int64_t max_delta_index = 0;

    {
        int64_t previous_index = 0;
        for (auto const& t : handler.triangles()) {
            int64_t const a = int64_t(t.i[0]);

            max_index = std::max(max_index, a);

            int64_t delta_index = a - previous_index;

            max_delta_index = std::max(std::abs(delta_index), max_delta_index);

            int64_t const b = int64_t(t.i[1]);

            max_index = std::max(max_index, b);

            delta_index = b - a;

            max_delta_index = std::max(std::abs(delta_index), max_delta_index);

            int64_t const c = int64_t(t.i[2]);

            max_index = std::max(max_index, c);

            delta_index = c - b;

            max_delta_index = std::max(std::abs(delta_index), max_delta_index);

            previous_index = c;
        }
    }

    bool   delta_indices = false;
    size_t index_bytes   = 4;

    if (max_delta_index < 0x0000000000008000) {
        delta_indices = true;
        index_bytes   = 2;
    } else if (max_index <= 0x000000000000FFFF) {
        index_bytes = 2;

    } else if (max_delta_index < 0x0000000080000000) {
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
        // If no UVs were loaded, explicitely set them to zero.
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

struct Cluster {
    Cluster() = default;

    Cluster(uint32_t part, uint32_t mat, Index_triangle const& triangle, uint32_t index_id,
            std::vector<Vertex> const& vertices) {
        original_part = part;

        material = mat;

        triangles.push_back(triangle);
        index_ids.push_back(index_id);

        float3 const a = float3(vertices[triangle.i[0]].p);
        float3 const b = float3(vertices[triangle.i[1]].p);
        float3 const c = float3(vertices[triangle.i[2]].p);

        center = (a + b + c) / 3.f;
    }

    bool insert_if_fits(uint32_t mat, Index_triangle const& triangle, uint32_t index_id,
                        std::vector<Vertex> const& vertices) {
        if (mat != material) {
            return false;
        }

        static float constexpr cr = 0.5f;

        float3 const a = float3(vertices[triangle.i[0]].p);
        float3 const b = float3(vertices[triangle.i[1]].p);
        float3 const c = float3(vertices[triangle.i[2]].p);

        if (distance(a, center) < cr && distance(b, center) < cr && distance(c, center) < cr) {
            triangles.push_back(triangle);
            index_ids.push_back(index_id);

            center = 0.5f * (center + ((a + b + c) / 3.f));

            return true;
        }

        return false;
    }

    bool in_range(uint32_t start, uint32_t num_indices) const {
        uint32_t const end = start + num_indices;

        for (uint32_t const i : index_ids) {
            if (i < start || i >= end) {
                return false;
            }
        }

        return true;
    }

    uint32_t original_part = 0xFFFFFFFF;

    uint32_t material = 0xFFFFFFFF;

    std::vector<Index_triangle> triangles;

    std::vector<uint32_t> index_ids;

    float3 center = float3(0.f);
};

void Exporter::write_json(std::string const& filename, Json_handler& handler) {
    // 80, 99, 100, 101, 102, 103

    std::vector<uint32_t> lms = {80, 99, 100, 101, 102, 103};

    std::string const out_name = string::extract_filename(filename) + ".json";

    std::cout << "Export " << out_name << std::endl;

    std::ofstream stream(out_name, std::ios::binary);

    if (!stream) {
        return;
    }

    stream << "{\n";

    stream << "\t\"geometry\": {\n";

    // Parts
    stream << "\t\t\"parts\": [\n";

    std::vector<Cluster> clusters;

    for (uint32_t pi = 0, plen = uint32_t(handler.parts().size()); pi < plen; ++pi) {
        auto const& p = handler.parts()[pi];

        bool fit = false;

        for (auto const lm : lms) {
            if (lm == p.material_index) {
                fit = true;
                continue;
            }
        }

        if (!fit) {
            continue;
        }

        for (size_t i = p.start_index, end = p.start_index + p.num_indices; i < end; i += 3) {
            Index_triangle const& tri = handler.triangles()[i / 3];

            bool hit = false;

            for (auto& c : clusters) {
                if (c.insert_if_fits(p.material_index, tri, uint32_t(i), handler.vertices())) {
                    hit = true;
                    break;
                }
            }

            if (!hit) {
                clusters.emplace_back(pi, p.material_index, tri, uint32_t(i), handler.vertices());
            }
        }
    }

    std::cout << "#Clusters: " << clusters.size() << std::endl;

    for (auto const& c : clusters) {
        auto const& p = handler.parts()[c.original_part];

        bool const in_range = c.in_range(p.start_index, uint32_t(c.triangles.size()) * 3);
        std::cout << c.material << ": " << c.triangles.size() << " " << in_range << std::endl;
    }

    auto const& parts = handler.parts();

    for (size_t i = 0, len = parts.size(); i < len; ++i) {
        /*if (80 == parts[i].material_index || 99 == parts[i].material_index ||
            100 == parts[i].material_index || 101 == parts[i].material_index ||
            102 == parts[i].material_index || 103 == parts[i].material_index) {
            uint32_t start = parts[i].start_index;

            for (auto const& c : clusters) {
                if (parts[i].material_index == c.material) {
                    uint32_t const num_indices = uint32_t(c.triangles.size()) * 3;

                    stream << "\t\t\t{" << std::endl;

                    stream << "\t\t\t\t\"material_index\": ";
                    stream << (num_indices > 300 ? c.material : 0);
                    stream << ",\n";

                    stream << "\t\t\t\t\"start_index\": ";
                    stream << start;
                    stream << ",\n";

                    if (num_indices < 600) {
                        std::cout << num_indices << std::endl;
                    }

                    stream << "\t\t\t\t\"num_indices\": ";
                    stream << num_indices;

                    stream << "\n\t\t\t}";

                    start += num_indices;

                    stream << ",\n";
                }
            }
        } else*/
        {
            stream << "\t\t\t{" << std::endl;

            stream << "\t\t\t\t\"material_index\": ";
            stream << parts[i].material_index;
            stream << ",\n";

            stream << "\t\t\t\t\"start_index\": ";
            stream << parts[i].start_index;
            stream << ",\n";

            stream << "\t\t\t\t\"num_indices\": ";
            stream << parts[i].num_indices;

            stream << "\n\t\t\t}";

            if (i < len - 1) {
                stream << ",\n";
            }
        }
    }

    stream << "\n\t\t],\n\n";

    // Primitive Topology
    stream << "\t\t\"primitive_topology\": \"triangle_list\",\n\n";

    stream << "\t\t\"vertices\": {\n";

    // Positions

    auto const& vertices = handler.vertices();

    if (handler.has_positions()) {
        stream << "\t\t\t\"positions\": [\n";

        stream << "\t\t\t\t";

        for (size_t i = 0, len = vertices.size(); i < len; ++i) {
            stream << vertices[i].p[0] << "," << vertices[i].p[1] << "," << vertices[i].p[2];

            if (i < len - 1) {
                stream << ",";
            }

            if ((i + 1) % 8 == 0) {
                stream << "\n\t\t\t\t";
            }
        }

        stream << "\n\t\t\t],\n\n";
    }

    // Texture_2D Coordinates
    if (handler.has_texture_coordinates()) {
        stream << "\t\t\t\"texture_coordinates_0\": [\n";

        stream << "\t\t\t\t";

        for (size_t i = 0, len = vertices.size(); i < len; ++i) {
            stream << vertices[i].uv[0] << "," << vertices[i].uv[1];

            if (i < len - 1) {
                stream << ",";
            }

            if ((i + 1) % 8 == 0) {
                stream << "\n\t\t\t\t";
            }
        }

        stream << "\n\t\t\t],\n\n";
    }

    // Normals
    if (handler.has_normals()) {
        stream << "\t\t\t\"normals\": [\n";

        stream << "\t\t\t\t";

        for (size_t i = 0, len = vertices.size(); i < len; ++i) {
            stream << vertices[i].n[0] << "," << vertices[i].n[1] << "," << vertices[i].n[2];

            if (i < len - 1) {
                stream << ",";
            }

            if ((i + 1) % 8 == 0) {
                stream << "\n\t\t\t\t";
            }
        }

        stream << "\n\t\t\t]";

        if (handler.has_tangents()) {
            stream << ",";
        }

        stream << "\n\n";
    }

    // Tangent Space
    if (handler.has_tangents()) {
        // Tangents
        stream << "\t\t\t\"tangents_and_bitangent_signs\": [\n";

        stream << "\t\t\t\t";

        for (size_t i = 0, len = vertices.size(); i < len; ++i) {
            stream << vertices[i].t[0] << "," << vertices[i].t[1] << "," << vertices[i].t[2] << ","
                   << (vertices[i].bitangent_sign ? -1.f : 1.f);

            if (i < len - 1) {
                stream << ",";
            }

            if ((i + 1) % 8 == 0) {
                stream << "\n\t\t\t\t";
            }
        }

        stream << "\n\t\t\t]\n\n";
    }

    stream << "\t\t},\n\n";

    // Indices
    stream << "\t\t\"indices\": [\n";

    stream << "\t\t\t";

    auto const& indices = handler.triangles();

    for (size_t i = 0, len = handler.triangles().size(); i < len; ++i) {
        stream << indices[i].i[0] << "," << indices[i].i[1] << "," << indices[i].i[2];

        if (i < len - 1) {
            stream << ",";
        }

        if ((i + 1) % 8 == 0) {
            stream << "\n\t\t\t";
        }
    }

    stream << "\n\t\t]\n";

    stream << "\t}\n";

    stream << "}";
}

}  // namespace scene::shape::triangle
