#include "triangle_mesh_provider.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/vector3.inl"
#include "base/memory/unique.inl"
#include "base/memory/variant_map.inl"
#include "bvh/triangle_bvh_builder_sah.inl"
#include "file/file.hpp"
#include "file/file_system.hpp"
#include "logging/logging.hpp"
#include "rapidjson/istreamwrapper.h"
#include "resource/resource_manager.hpp"
#include "resource/resource_provider.inl"
#include "triangle_json_handler.hpp"
#include "triangle_mesh.hpp"
#include "triangle_mesh_exporter.hpp"
#include "triangle_morph_target_collection.hpp"
#include "triangle_morphable_mesh.hpp"
#include "triangle_primitive.hpp"

#include "base/debug/assert.hpp"
#ifdef SU_DEBUG
#include <iostream>
#include "base/math/print.hpp"
#endif

namespace scene::shape::triangle {

#ifdef SU_DEBUG
bool check(const std::vector<Vertex>& vertices, std::string const& filename);
bool check_and_fix(std::vector<Vertex>& vertices, std::string const& filename);
#endif

Provider::Provider() noexcept {}

Provider::~Provider() noexcept {}

Shape* Provider::load(std::string const& filename, memory::Variant_map const& /*options*/,
                      resource::Manager& manager, std::string& resolved_name) noexcept {
    auto stream_pointer = manager.filesystem().read_stream(filename, resolved_name);
    if (!stream_pointer) {
        logging::error("Loading mesh %S: ", filename);
        return nullptr;
    }

    file::Type const type = file::query_type(*stream_pointer);
    if (file::Type::SUB == type) {
        Shape* mesh = load_binary(*stream_pointer, manager.thread_pool());
        if (!mesh) {
            logging::error("Loading mesh %S: ", filename);
        }

        return mesh;
    }

    memory::Unique_ptr<Json_handler> handler(new Json_handler);

    {
        rapidjson::IStreamWrapper json_stream(*stream_pointer);

        rapidjson::Reader reader;

        reader.Parse(json_stream, *handler);

        delete stream_pointer.release();
    }

    if (!handler->morph_targets().empty()) {
        return load_morphable_mesh(filename, handler->morph_targets(), manager);
    }

    if (handler->vertices().empty()) {
        logging::error("Mesh %S does not contain vertices.", filename);
        return nullptr;
    }

    if (!handler->has_positions()) {
        logging::error("Mesh %S does not contain vertex positions.", filename);
        return nullptr;
    }

    if (handler->triangles().empty()) {
        logging::error("Mesh %S does not contain indices.", filename);
        return nullptr;
    }

    if (handler->parts().empty()) {
        handler->create_part();
    }

    if (!handler->has_normals()) {
        // If no normals were loaded, assign something.
        // Might be smarter to throw an exception,
        // or just go ahead and actually compute the geometry normal...
        for (auto& v : handler->vertices()) {
            v.n = packed_float3(0.f, 1.f, 0.f);
        }
    }

    if (!handler->has_tangents()) {
        // If no tangents were loaded, compute the tangent space manually
        for (auto& v : handler->vertices()) {
            packed_float3 b;
            orthonormal_basis(v.n, v.t, b);
            v.bitangent_sign = 0;
        }
    }

    SOFT_ASSERT(check_and_fix(handler->vertices(), filename));

    // Exporter::write(filename, *handler);

    auto mesh = new Mesh;

    mesh->allocate_parts(uint32_t(handler->parts().size()));

    for (uint32_t p = 0, len = uint32_t(handler->parts().size()); p < len; ++p) {
        mesh->set_material_for_part(p, handler->parts()[p].material_index);
    }

    manager.thread_pool().run_async([mesh, handler_raw{handler.release()}, &manager]() {
        logging::verbose("Started asynchronously building triangle mesh BVH.");

        auto& triangles = handler_raw->triangles();

        uint32_t part = 0;
        for (auto const& p : handler_raw->parts()) {
            uint32_t const triangles_start = p.start_index / 3;
            uint32_t const triangles_end   = (p.start_index + p.num_indices) / 3;

            for (uint32_t i = triangles_start; i < triangles_end; ++i) {
                triangles[i].material_index = part;
            }

            ++part;
        }

        auto const& vertices = handler_raw->vertices();

        Vertex_stream_interleaved vertex_stream(uint32_t(vertices.size()), vertices.data());

        build_bvh(*mesh, uint32_t(triangles.size()), triangles.data(), vertex_stream,
                  manager.thread_pool());

        delete handler_raw;

        logging::verbose("Finished asynchronously building triangle mesh BVH.");
    });

    return mesh;
}

Shape* Provider::load(void const* /*data*/, std::string const& /*source_name*/,
                      memory::Variant_map const& /*options*/,
                      resource::Manager& /*manager*/) noexcept {
    return nullptr;
}

size_t Provider::num_bytes() const noexcept {
    return sizeof(*this);
}

size_t Provider::num_bytes(Shape const* resource) const noexcept {
    return resource->num_bytes();
}

Shape* Provider::create_mesh(Triangles const& triangles, Vertices const& vertices,
                             uint32_t num_parts, thread::Pool& thread_pool) noexcept {
    if (triangles.empty() || vertices.empty() || !num_parts) {
        logging::error("No mesh data.");
        return nullptr;
    }

    auto mesh = new Mesh;

    mesh->allocate_parts(num_parts);

    for (uint32_t i = 0; i < num_parts; ++i) {
        mesh->set_material_for_part(i, i);
    }

    thread_pool.run_async([mesh, triangles_in{std::move(triangles)},
                           vertices_in{std::move(vertices)}, &thread_pool]() {
        Vertex_stream_interleaved vertex_stream(uint32_t(vertices_in.size()), vertices_in.data());

        build_bvh(*mesh, uint32_t(triangles_in.size()), triangles_in.data(), vertex_stream,
                  thread_pool);
    });

    //	build_bvh(*mesh, triangles, vertices, bvh_preset, thread_pool);

    return mesh;
}

Shape* Provider::load_morphable_mesh(std::string const& filename, Strings const& morph_targets,
                                     resource::Manager& manager) noexcept {
    auto collection = new Morph_target_collection;

    Json_handler handler;

    for (auto& targets : morph_targets) {
        auto stream_pointer = manager.filesystem().read_stream(targets);
        if (!stream_pointer) {
            continue;
        }

        rapidjson::IStreamWrapper json_stream(*stream_pointer);

        handler.clear(collection->triangles().empty());

        rapidjson::Reader reader;

        reader.Parse(json_stream, handler);

        if (!handler.has_positions()) {
            continue;
        }

        if (!handler.has_normals()) {
            // If no normals were loaded assign identity
            // Might be smarter to throw an exception
            for (auto& v : handler.vertices()) {
                v.n = packed_float3(0.f, 0.f, 1.f);
            }
        }

        if (!handler.has_tangents()) {
            // If no tangents were loaded, compute the tangent space manually
            for (auto& v : handler.vertices()) {
                packed_float3 b;
                orthonormal_basis(v.n, v.t, b);
                v.bitangent_sign = 0;
            }
        }

        // The idea is to have one identical set of indices for all morph targets
        if (collection->triangles().empty()) {
            auto& triangles = handler.triangles();

            uint32_t part = 0;
            for (auto& p : handler.parts()) {
                uint32_t triangles_start = p.start_index / 3;
                uint32_t triangles_end   = (p.start_index + p.num_indices) / 3;

                for (uint32_t i = triangles_start; i < triangles_end; ++i) {
                    uint32_t a = triangles[i].i[0];
                    uint32_t b = triangles[i].i[1];
                    uint32_t c = triangles[i].i[2];

                    collection->triangles().emplace_back(a, b, c, part);
                }

                ++part;
            }
        }

        (void)filename;
        SOFT_ASSERT(check(handler.vertices(), filename));

        collection->add_swap_vertices(handler.vertices());
    }

    if (collection->triangles().empty()) {
        return nullptr;
    }

    uint32_t const num_parts = uint32_t(handler.parts().size());

    auto mesh = new Morphable_mesh(collection, num_parts);

    return mesh;
}

void Provider::build_bvh(Mesh& mesh, uint32_t num_triangles, Index_triangle const* const triangles,
                         Vertex_stream const& vertices, thread::Pool& thread_pool) noexcept {
    bvh::Builder_SAH builder(16, 64);
    builder.build(mesh.tree(), num_triangles, triangles, vertices, 4, thread_pool);
}

template <typename Index>
void fill_triangles_delta(uint32_t num_parts, Part const* const parts, Index const* const indices,
                          Index_triangle* const triangles) noexcept {
    int32_t previous_index(0);

    for (uint32_t i = 0; i < num_parts; ++i) {
        Part const& p = parts[i];

        uint32_t const triangles_start = p.start_index / 3;
        uint32_t const triangles_end   = (p.start_index + p.num_indices) / 3;

        for (uint32_t j = triangles_start; j < triangles_end; ++j) {
            Index_triangle& t = triangles[j];

            int32_t const a = previous_index + int32_t(indices[j * 3 + 0]);
            t.i[0]          = uint32_t(a);

            int32_t const b = a + int32_t(indices[j * 3 + 1]);
            t.i[1]          = uint32_t(b);

            int32_t const c = b + int32_t(indices[j * 3 + 2]);
            t.i[2]          = uint32_t(c);

            previous_index = c;

            t.material_index = i;
        }
    }
}

template <typename Index>
void fill_triangles(uint32_t num_parts, Part const* const parts, Index const* const indices,
                    Index_triangle* const triangles) noexcept {
    for (uint32_t i = 0; i < num_parts; ++i) {
        Part const& p = parts[i];

        uint32_t const triangles_start = p.start_index / 3;
        uint32_t const triangles_end   = (p.start_index + p.num_indices) / 3;

        for (uint32_t j = triangles_start; j < triangles_end; ++j) {
            Index_triangle& t = triangles[j];

            t.i[0] = uint32_t(indices[j * 3 + 0]);
            t.i[1] = uint32_t(indices[j * 3 + 1]);
            t.i[2] = uint32_t(indices[j * 3 + 2]);

            t.material_index = i;
        }
    }
}

Shape* Provider::load_binary(std::istream& stream, thread::Pool& thread_pool) noexcept {
    stream.seekg(4);

    uint64_t json_size = 0;
    stream.read(reinterpret_cast<char*>(&json_size), sizeof(uint64_t));

    memory::Array<char> json_string(uint32_t(json_size) + 1);
    stream.read(json_string.data(), std::streamsize(json_size * sizeof(char)));
    json_string[uint32_t(json_size)] = 0;

    std::string error;
    auto const  root = json::parse_insitu(json_string.data(), error);
    if (!root) {
        logging::push_error("Shape: " + error);
        return nullptr;
    }

    json::Value::ConstMemberIterator const geometry_node = root->FindMember("geometry");
    if (root->MemberEnd() == geometry_node) {
        logging::push_error("Model has no geometry node.");
        return nullptr;
    }

    json::Value const& geometry_value = geometry_node->value;

    uint32_t num_parts = 0;
    Part*    parts     = nullptr;

    uint64_t vertices_offset = 0;
    uint64_t vertices_size   = 0;

    uint64_t indices_offset = 0;
    uint64_t indices_size   = 0;
    uint64_t index_bytes    = 0;

    uint32_t num_vertices = 0;
    uint32_t num_indices  = 0;

    bool delta_indices = false;

    bool interleaved_vertex_stream = false;

    bool has_uvs      = false;
    bool has_tangents = false;

    for (auto& n : geometry_value.GetObject()) {
        if ("parts" == n.name) {
            num_parts = n.value.Size();

            parts = new Part[num_parts];

            uint32_t i = 0;

            for (auto const& pn : n.value.GetArray()) {
                Part& p = parts[i];

                p.start_index    = json::read_uint(pn, "start_index");
                p.num_indices    = json::read_uint(pn, "num_indices");
                p.material_index = json::read_uint(pn, "material_index");

                ++i;
            }
        } else if ("vertices" == n.name) {
            for (auto const& vn : n.value.GetObject()) {
                if ("binary" == vn.name) {
                    vertices_offset = json::read_uint(vn.value, "offset");
                    vertices_size   = json::read_uint(vn.value, "size");
                } else if ("num_vertices" == vn.name) {
                    num_vertices = json::read_uint(vn.value);
                } else if ("layout" == vn.name) {
                    for (auto const& ln : vn.value.GetArray()) {
                        std::string const semantic_name = json::read_string(ln, "semantic_name");
                        if ("Tangent" == semantic_name) {
                            has_tangents = true;
                        } else if ("Texture_coordinate" == semantic_name) {
                            has_uvs = true;
                        } else if ("Bitangent_sign" == semantic_name) {
                            if ("UInt8" != json::read_string(ln, "encoding")) {
                                logging::push_error("Bitangent_sign must be encoded as UInt8.");
                                return nullptr;
                            }

                            if (0 == json::read_uint(ln, "stream")) {
                                interleaved_vertex_stream = true;
                            }
                        }
                    }
                }
            }
        } else if ("indices" == n.name) {
            for (auto const& in : n.value.GetObject()) {
                if ("binary" == in.name) {
                    indices_offset = json::read_uint(in.value, "offset");
                    indices_size   = json::read_uint(in.value, "size");
                } else if ("num_indices" == in.name) {
                    num_indices = json::read_uint(in.value);
                } else if ("encoding" == in.name) {
                    if ("Int16" == json::read_string(in.value)) {
                        index_bytes   = 2;
                        delta_indices = true;
                    } else if ("UInt16" == json::read_string(in.value)) {
                        index_bytes   = 2;
                        delta_indices = false;
                    } else if ("Int32" == json::read_string(in.value)) {
                        index_bytes   = 4;
                        delta_indices = true;
                    } else {
                        index_bytes   = 4;
                        delta_indices = false;
                    }
                }
            }
        }
    }

    json_string.release();

    bool const has_uvs_and_tangents = has_uvs && has_tangents;

    uint64_t const binary_start = json_size + 4u + sizeof(uint64_t);

    if (0 == num_vertices) {
        // Handle legacy files, that curiously worked because of Gzip_stream bug!
        // (seekg() was not implemented properly)
        num_vertices = uint32_t(vertices_size / sizeof(Vertex));

        if (!interleaved_vertex_stream) {
            indices_offset = num_vertices * Vertex::unpadded_size();
        }
    }

    stream.seekg(std::streamoff(binary_start + vertices_offset));

    Vertex_stream* vertex_stream = nullptr;

    if (interleaved_vertex_stream) {
        Vertex* vertices = new Vertex[num_vertices];

        stream.read(reinterpret_cast<char*>(vertices), std::streamsize(vertices_size));

        vertex_stream = new Vertex_stream_interleaved(num_vertices, vertices);
    } else {
        packed_float3* p = new packed_float3[num_vertices];
        packed_float3* n = new packed_float3[num_vertices];

        stream.read(reinterpret_cast<char*>(p), num_vertices * sizeof(packed_float3));
        stream.read(reinterpret_cast<char*>(n), num_vertices * sizeof(packed_float3));

        if (has_uvs_and_tangents) {
            packed_float3* t   = new packed_float3[num_vertices];
            float2*        uv  = new float2[num_vertices];
            uint8_t*       bts = new uint8_t[num_vertices];

            stream.read(reinterpret_cast<char*>(t), num_vertices * sizeof(packed_float3));
            stream.read(reinterpret_cast<char*>(uv), num_vertices * sizeof(float2));
            stream.read(reinterpret_cast<char*>(bts), num_vertices * sizeof(uint8_t));

            vertex_stream = new Vertex_stream_separate(num_vertices, p, n, t, uv, bts);
        } else {
            vertex_stream = new Vertex_stream_separate_compact(num_vertices, p, n);
        }
    }

    if (0 == num_indices) {
        num_indices = uint32_t(indices_size / index_bytes);
    }

    char* indices = new char[indices_size];

    stream.seekg(std::streamoff(binary_start + indices_offset));
    stream.read(indices, std::streamsize(indices_size));

    auto mesh = new Mesh;

    mesh->allocate_parts(num_parts);

    for (uint32_t p = 0; p < num_parts; ++p) {
        mesh->set_material_for_part(p, parts[p].material_index);
    }

    thread_pool.run_async([mesh, num_parts, parts, num_indices, indices, vertex_stream, index_bytes,
                           delta_indices, &thread_pool]() {
        memory::Array<Index_triangle> triangles(num_indices / 3);

        if (4 == index_bytes) {
            if (delta_indices) {
                int32_t const* indices32 = reinterpret_cast<int32_t const*>(indices);
                fill_triangles_delta(num_parts, parts, indices32, triangles.data());
            } else {
                uint32_t const* indices32 = reinterpret_cast<uint32_t const*>(indices);
                fill_triangles(num_parts, parts, indices32, triangles.data());
            }
        } else {
            if (delta_indices) {
                int16_t const* indices16 = reinterpret_cast<int16_t const*>(indices);
                fill_triangles_delta(num_parts, parts, indices16, triangles.data());
            } else {
                uint16_t const* indices16 = reinterpret_cast<uint16_t const*>(indices);
                fill_triangles(num_parts, parts, indices16, triangles.data());
            }
        }

        delete[] indices;
        delete[] parts;

        build_bvh(*mesh, uint32_t(triangles.size()), triangles.data(), *vertex_stream, thread_pool);

        vertex_stream->release();

        delete vertex_stream;
    });

    return mesh;
}

#ifdef SU_DEBUG
bool check(const std::vector<Vertex>& vertices, std::string const& filename) {
    for (size_t i = 0, len = vertices.size(); i < len; ++i) {
        auto const& v = vertices[i];
        if (squared_length(v.n) < 0.1f || squared_length(v.t) < 0.1f) {
            std::cout << filename << " vertex " << i << std::endl;

            std::cout << "n: " << v.n << std::endl;
            std::cout << "t: " << v.t << std::endl;

            return false;
        }
    }

    return true;
}

bool check_and_fix(std::vector<Vertex>& vertices, std::string const& /*filename*/) {
    bool success = true;

    for (size_t i = 0, len = vertices.size(); i < len; ++i) {
        auto& v = vertices[i];

        if (squared_length(v.n) < 0.1f) {
            //		std::cout << filename << " vertex " << i << std::endl;

            std::cout << "n: " << v.n << " converted to ";
            v.n = packed_float3(0.f, 1.f, 0.f);
            std::cout << v.n << std::endl;

            success = false;
        }

        if (squared_length(v.t) < 0.1f) {
            //	std::cout << filename << " vertex " << i << std::endl;

            std::cout << "t: " << v.t << " converted to ";
            v.t = packed_float3(math::tangent(float3(v.n)));
            std::cout << v.t << std::endl;

            success = false;
        }
    }

    return success;
}
#endif

}  // namespace scene::shape::triangle
