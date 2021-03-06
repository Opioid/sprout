#include "triangle_mesh_provider.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"
#include "base/memory/buffer.hpp"
#include "base/thread/thread_pool.hpp"
#include "bvh/triangle_bvh_builder_sah.hpp"
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
#include "base/chrono/chrono.hpp"
#include "base/math/print.inl"

#include <iostream>
#endif

namespace scene::shape::triangle {

#ifdef SU_DEBUG
bool check(std::vector<Vertex> const& vertices, std::string const& filename);

bool check_and_fix(std::vector<serialize::Part>& parts, std::vector<Index_triangle>& triangles,
                   std::vector<Vertex>& vertices, std::string const& filename);
#endif

Provider::Provider() = default;

Provider::~Provider() = default;

Shape* Provider::load(std::string const& filename, Variants const& /*options*/,
                      Resources& resources, std::string& resolved_name) {
    auto stream = resources.filesystem().read_stream(filename, resolved_name);
    if (!stream) {
        logging::error("Loading mesh %S: ", filename);
        return nullptr;
    }

    if (file::Type::SUB == file::query_type(*stream)) {
        Shape* mesh = load_binary(*stream, resources.threads());
        if (!mesh) {
            logging::error("Loading mesh %S: ", filename);
        }

        return mesh;
    }

#ifdef SU_DEBUG
    auto const loading_start = std::chrono::high_resolution_clock::now();
#endif

    serialize::Json_handler handler;

    {
        static size_t constexpr Buffer_size = 8192;

        memory::Buffer<char> buffer(Buffer_size);

        rapidjson::IStreamWrapper json_stream(*stream, buffer.data(), Buffer_size);

        rapidjson::Reader reader;

        reader.Parse(json_stream, handler);

        stream.close();
    }

    if (!handler.morph_targets().empty()) {
        return load_morphable_mesh(filename, handler.morph_targets(), resources);
    }

    if (handler.vertices().empty()) {
        logging::error("Mesh %S does not contain vertices.", filename);
        return nullptr;
    }

    if (!handler.has_positions()) {
        logging::error("Mesh %S does not contain vertex positions.", filename);
        return nullptr;
    }

    if (handler.triangles().empty()) {
        logging::error("Mesh %S does not contain indices.", filename);
        return nullptr;
    }

    if (handler.parts().empty()) {
        handler.create_part();
    }

    if (!handler.has_normals()) {
        // If no normals were loaded, compute geometry normal.
        for (auto const& t : handler.triangles()) {
            float3 const a = float3(handler.vertices()[t.i[0]].p);
            float3 const b = float3(handler.vertices()[t.i[1]].p);
            float3 const c = float3(handler.vertices()[t.i[2]].p);

            float3 const e1 = b - a;
            float3 const e2 = c - a;

            float3 const n = normalize(cross(e1, e2));

            handler.vertices()[t.i[0]].n = packed_float3(n);
            handler.vertices()[t.i[1]].n = packed_float3(n);
            handler.vertices()[t.i[2]].n = packed_float3(n);
        }
    }

    if (!handler.has_tangents()) {
        // If no tangents were loaded, compute some tangent space manually
        for (auto& v : handler.vertices()) {
            v.t = packed_float3(tangent(float3(v.n)));

            v.bitangent_sign = 0;
        }
    }

    SOFT_ASSERT(check_and_fix(handler.parts(), handler.triangles(), handler.vertices(), filename));

    // Exporter::write(filename, *handler);

    auto mesh = new Mesh;

    uint32_t const num_parts = uint32_t(handler.parts().size());

    mesh->allocate_parts(num_parts);

    for (uint32_t p = 0; p < num_parts; ++p) {
        mesh->set_material_for_part(p, handler.parts()[p].material_index);
    }

    LOGGING_VERBOSE("Parsing mesh %f s", chrono::seconds_since(loading_start));

    resources.threads().run_async([mesh, handler{std::move(handler)},
                                   &resources]() mutable noexcept {
        LOGGING_VERBOSE("Started asynchronously building triangle mesh BVH.");

        auto& triangles = handler.triangles();

        uint32_t part = 0;
        for (auto const& p : handler.parts()) {
            uint32_t const triangles_start = p.start_index / 3;
            uint32_t const triangles_end   = (p.start_index + p.num_indices) / 3;

            for (uint32_t i = triangles_start; i < triangles_end; ++i) {
                triangles[i].part = part;
            }

            ++part;
        }

        auto const& vertices = handler.vertices();

        Vertex_stream_interleaved const vertex_stream(uint32_t(vertices.size()), vertices.data());

        build_bvh(*mesh, uint32_t(triangles.size()), triangles.data(), vertex_stream,
                  resources.threads());

        LOGGING_VERBOSE("Finished asynchronously building triangle mesh BVH.");
    });

    return mesh;
}

Shape* Provider::load(void const* data, std::string const& /*source_name*/,
                      Variants const& /*options*/, Resources& resources) {
    Description const& desc = *reinterpret_cast<Description const*>(data);

    if (!desc.positions || !desc.normals) {
        logging::error("Mesh does not contain vertex positions or normals.");
        return nullptr;
    }

    auto mesh = new Mesh;

    uint32_t const num_parts = desc.parts ? desc.num_parts : 1;

    mesh->allocate_parts(num_parts);

    if (desc.parts) {
        for (uint32_t p = 0; p < num_parts; ++p) {
            mesh->set_material_for_part(p, desc.parts[p * 3 + 2]);
        }
    } else {
        mesh->set_material_for_part(0, 0);
    }

    resources.threads().run_async([mesh, desc, &resources]() noexcept {
        LOGGING_VERBOSE("Started asynchronously building triangle mesh BVH.");

        uint32_t const num_triangles = desc.num_triangles;

        memory::Buffer<Index_triangle> triangles(num_triangles);

        uint32_t const empty_part[] = {0, num_triangles * 3, 0};

        uint32_t const* parts = desc.parts ? desc.parts : empty_part;

        for (uint32_t p = 0, len = desc.parts ? desc.num_parts : 1; p < len; ++p) {
            uint32_t const start_index = parts[p * 3 + 0];
            uint32_t const num_indices = parts[p * 3 + 1];

            uint32_t const triangles_start = start_index / 3;
            uint32_t const triangles_end   = (start_index + num_indices) / 3;

            if (!desc.indices) {
                for (uint32_t i = triangles_start; i < triangles_end; ++i) {
                    triangles[i].i[0] = i * 3 + 0;
                    triangles[i].i[1] = i * 3 + 1;
                    triangles[i].i[2] = i * 3 + 2;

                    triangles[i].part = p;
                }
            } else {
                for (uint32_t i = triangles_start; i < triangles_end; ++i) {
                    triangles[i].i[0] = desc.indices[i * 3 + 0];
                    triangles[i].i[1] = desc.indices[i * 3 + 1];
                    triangles[i].i[2] = desc.indices[i * 3 + 2];

                    triangles[i].part = p;
                }
            }
        }

        memory::Buffer<packed_float3> tangents;

        if (!desc.tangents) {
            tangents.resize(desc.num_vertices);

            packed_float3 const* normals = reinterpret_cast<packed_float3 const*>(desc.normals);

            for (uint32_t i = 0, len = desc.num_vertices; i < len; ++i) {
                tangents[i] = packed_float3(tangent(float3(normals[i])));
            }
        }

        float2 const empty_uv(0.f);

        Vertex_stream_CAPI const vertex_stream(
            desc.num_vertices, desc.positions_stride, desc.normals_stride,
            tangents ? 3 : desc.tangents_stride, desc.uvs ? desc.uvs_stride : 0, desc.positions,
            desc.normals, tangents ? tangents[0].v : desc.tangents,
            desc.uvs ? desc.uvs : empty_uv.v);

        build_bvh(*mesh, num_triangles, triangles, vertex_stream, resources.threads());

        LOGGING_VERBOSE("Finished asynchronously building triangle mesh BVH.");
    });

    return mesh;
}

Shape* Provider::load_morphable_mesh(std::string const& filename, Strings const& morph_targets,
                                     Resources& resources) {
    Morph_target_collection collection;

    serialize::Json_handler handler;

    static size_t constexpr Buffer_size = 8192;

    memory::Buffer<char> buffer(Buffer_size);

    for (auto& targets : morph_targets) {
        auto stream = resources.filesystem().read_stream(targets);
        if (!stream) {
            continue;
        }

        handler.clear(collection.triangles().empty());

        rapidjson::IStreamWrapper json_stream(*stream, buffer.data(), Buffer_size);

        rapidjson::Reader reader;

        reader.Parse(json_stream, handler);

        stream.close();

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
                v.t = packed_float3(tangent(float3(v.n)));

                v.bitangent_sign = 0;
            }
        }

        // The idea is to have one identical set of indices for all morph targets
        if (collection.triangles().empty()) {
            auto& triangles = handler.triangles();

            uint32_t part = 0;
            for (auto& p : handler.parts()) {
                uint32_t triangles_start = p.start_index / 3;
                uint32_t triangles_end   = (p.start_index + p.num_indices) / 3;

                for (uint32_t i = triangles_start; i < triangles_end; ++i) {
                    uint32_t const a = triangles[i].i[0];
                    uint32_t const b = triangles[i].i[1];
                    uint32_t const c = triangles[i].i[2];

                    collection.triangles().emplace_back(a, b, c, part);
                }

                ++part;
            }
        }

        (void)filename;
        SOFT_ASSERT(check(handler.vertices(), filename));

        collection.add_swap_vertices(handler.vertices());
    }

    if (collection.triangles().empty()) {
        return nullptr;
    }

    uint32_t const num_parts = uint32_t(handler.parts().size());

    auto mesh = new Morphable_mesh(std::move(collection), num_parts);

    return mesh;
}

Shape* Provider::create_mesh(Triangles& triangles, Vertices& vertices, uint32_t num_parts,
                             Threads& threads) {
    if (triangles.empty() || vertices.empty() || !num_parts) {
        logging::error("No mesh data.");
        return nullptr;
    }

    auto mesh = new Mesh;

    mesh->allocate_parts(num_parts);

    for (uint32_t i = 0; i < num_parts; ++i) {
        mesh->set_material_for_part(i, i);
    }

    threads.run_async([mesh, triangles{std::move(triangles)}, vertices{std::move(vertices)},
                       &threads]() noexcept {
        Vertex_stream_interleaved vertex_stream(uint32_t(vertices.size()), vertices.data());

        build_bvh(*mesh, uint32_t(triangles.size()), triangles.data(), vertex_stream, threads);
    });

    return mesh;
}

void Provider::build_bvh(Mesh& mesh, uint32_t num_triangles, Index_triangle const* const triangles,
                         Vertex_stream const& vertices, Threads& threads) {
    bvh::Builder_SAH builder(16, 64, 4);
    builder.build(mesh.tree(), num_triangles, triangles, vertices, threads);
}

template <typename Index>
void fill_triangles_delta(uint32_t num_parts, serialize::Part const* const parts,
                          Index const* const indices, Index_triangle* const triangles) {
    int32_t previous_index(0);

    for (uint32_t i = 0; i < num_parts; ++i) {
        auto const& p = parts[i];

        uint32_t const triangles_start = p.start_index / 3;
        uint32_t const triangles_end   = (p.start_index + p.num_indices) / 3;

        for (uint32_t j = triangles_start; j < triangles_end; ++j) {
            Index_triangle& t = triangles[j];

            int32_t const a = previous_index + int32_t(indices[j * 3 + 0]);

            t.i[0] = uint32_t(a);

            int32_t const b = a + int32_t(indices[j * 3 + 1]);

            t.i[1] = uint32_t(b);

            int32_t const c = b + int32_t(indices[j * 3 + 2]);

            t.i[2] = uint32_t(c);

            previous_index = c;

            t.part = i;
        }
    }
}

template <typename Index>
void fill_triangles(uint32_t num_parts, serialize::Part const* const parts,
                    Index const* const indices, Index_triangle* const triangles) {
    for (uint32_t i = 0; i < num_parts; ++i) {
        auto const& p = parts[i];

        uint32_t const triangles_start = p.start_index / 3;
        uint32_t const triangles_end   = (p.start_index + p.num_indices) / 3;

        for (uint32_t j = triangles_start; j < triangles_end; ++j) {
            Index_triangle& t = triangles[j];

            t.i[0] = uint32_t(indices[j * 3 + 0]);
            t.i[1] = uint32_t(indices[j * 3 + 1]);
            t.i[2] = uint32_t(indices[j * 3 + 2]);

            t.part = i;
        }
    }
}

Shape* Provider::load_binary(std::istream& stream, Threads& threads) {
#ifdef SU_DEBUG
    auto const loading_start = std::chrono::high_resolution_clock::now();
#endif

    stream.seekg(4);

    uint64_t json_size = 0;
    stream.read(reinterpret_cast<char*>(&json_size), sizeof(uint64_t));

    memory::Buffer<char> json_string(json_size + 1);
    stream.read(json_string.data(), std::streamsize(json_size * sizeof(char)));
    json_string[json_size] = 0;

    rapidjson::Document root;
    root.ParseInsitu(json_string.data());
    if (root.HasParseError()) {
        logging::push_error("json: " + json::read_error(root));
        return nullptr;
    }

    json::Value::ConstMemberIterator const geometry_node = root.FindMember("geometry");
    if (root.MemberEnd() == geometry_node) {
        logging::push_error("Model has no geometry node.");
        return nullptr;
    }

    json::Value const& geometry_value = geometry_node->value;

    uint32_t num_parts = 0;

    memory::Buffer<serialize::Part> parts;

    uint64_t vertices_offset = 0;
    uint64_t vertices_size   = 0;

    uint64_t indices_offset = 0;
    uint64_t indices_size   = 0;
    uint64_t index_bytes    = 0;

    uint32_t num_vertices = 0;
    uint32_t num_indices  = 0;

    bool delta_indices = false;

    bool interleaved_vertex_stream = false;

    bool tangent_space_as_quaternion = false;
    bool has_uvs                     = false;
    bool has_tangents                = false;

    for (auto& n : geometry_value.GetObject()) {
        if ("parts" == n.name) {
            num_parts = n.value.Size();

            parts.resize(num_parts);

            uint32_t i = 0;

            for (auto const& pn : n.value.GetArray()) {
                auto& p = parts[i];

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
                        } else if ("Tangent_space" == semantic_name) {
                            tangent_space_as_quaternion = true;
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
        stream.read(reinterpret_cast<char*>(p), num_vertices * sizeof(packed_float3));

        if (tangent_space_as_quaternion) {
            Quaternion* ts = new Quaternion[num_vertices];
            float2*     uv = new float2[num_vertices];

            stream.read(reinterpret_cast<char*>(ts), num_vertices * sizeof(Quaternion));

            stream.read(reinterpret_cast<char*>(uv), num_vertices * sizeof(float2));

            vertex_stream = new Vertex_stream_separate_ts(num_vertices, p, ts, uv);
        } else {
            packed_float3* n = new packed_float3[num_vertices];
            stream.read(reinterpret_cast<char*>(n), num_vertices * sizeof(packed_float3));

            if (has_uvs_and_tangents) {
                packed_float3* t   = new packed_float3[num_vertices];
                float2*        uv  = new float2[num_vertices];
                uint8_t*       bts = new uint8_t[num_vertices];

                stream.read(reinterpret_cast<char*>(t), num_vertices * sizeof(packed_float3));
                stream.read(reinterpret_cast<char*>(uv), num_vertices * sizeof(float2));
                stream.read(reinterpret_cast<char*>(bts), num_vertices * sizeof(uint8_t));

                for (uint32_t i = 0; i < num_vertices; ++i) {
                    packed_float3& ti = t[i];

                    if (0.f == ti[0] && 0.f == ti[1] && 0.f == ti[2]) {
                        ti = packed_float3(tangent(float3(n[i])));
                    }
                }

                vertex_stream = new Vertex_stream_separate(num_vertices, p, n, t, uv, bts);
            } else {
                vertex_stream = new Vertex_stream_separate_compact(num_vertices, p, n);
            }
        }
    }

    if (0 == num_indices) {
        num_indices = uint32_t(indices_size / index_bytes);
    }

    memory::Buffer<char> indices(indices_size);

    stream.seekg(std::streamoff(binary_start + indices_offset));
    stream.read(indices, std::streamsize(indices_size));

    if (stream.gcount() < std::streamsize(indices_size)) {
        logging::push_error("Could not read all index data.");

        vertex_stream->release();

        delete vertex_stream;

        return nullptr;
    }

    auto mesh = new Mesh;

    mesh->allocate_parts(num_parts);

    for (uint32_t p = 0; p < num_parts; ++p) {
        auto const& part = parts[p];

        if (part.start_index + part.num_indices > num_indices) {
            logging::push_error("Part indices out of bounds.");

            delete mesh;

            vertex_stream->release();

            delete vertex_stream;

            return nullptr;
        }

        mesh->set_material_for_part(p, part.material_index);
    }

    LOGGING_VERBOSE("Parsing mesh %f s", chrono::seconds_since(loading_start));

    threads.run_async([mesh, num_parts, parts{std::move(parts)}, num_indices,
                       indices{std::move(indices)}, vertex_stream, index_bytes, delta_indices,
                       &threads]() noexcept {
        LOGGING_VERBOSE("Started asynchronously building triangle mesh BVH.");

        uint32_t const num_triangles = num_indices / 3;

        memory::Buffer<Index_triangle> triangles(num_triangles);

        if (4 == index_bytes) {
            if (delta_indices) {
                int32_t const* indices32 = reinterpret_cast<int32_t const*>(indices.data());
                fill_triangles_delta(num_parts, parts, indices32, triangles.data());
            } else {
                uint32_t const* indices32 = reinterpret_cast<uint32_t const*>(indices.data());
                fill_triangles(num_parts, parts, indices32, triangles.data());
            }
        } else {
            if (delta_indices) {
                int16_t const* indices16 = reinterpret_cast<int16_t const*>(indices.data());
                fill_triangles_delta(num_parts, parts, indices16, triangles.data());
            } else {
                uint16_t const* indices16 = reinterpret_cast<uint16_t const*>(indices.data());
                fill_triangles(num_parts, parts, indices16, triangles.data());
            }
        }

        build_bvh(*mesh, num_triangles, triangles.data(), *vertex_stream, threads);

        vertex_stream->release();

        delete vertex_stream;

        LOGGING_VERBOSE("Finished asynchronously building triangle mesh BVH.");
    });

    return mesh;
}

#ifdef SU_DEBUG
bool check(std::vector<Vertex> const& vertices, std::string const& filename) {
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

bool check_and_fix(std::vector<serialize::Part>& parts, std::vector<Index_triangle>& triangles,
                   std::vector<Vertex>& vertices, std::string const& /*filename*/) {
    bool success = true;

    std::vector<uint32_t> bad_triangles;

    for (uint32_t i = 0, len = uint32_t(triangles.size()); i < len; ++i) {
        auto const& tri = triangles[i];

        packed_float3 const a = vertices[tri.i[0]].p;
        packed_float3 const b = vertices[tri.i[1]].p;
        packed_float3 const c = vertices[tri.i[2]].p;

        packed_float3 const e1 = b - a;
        packed_float3 const e2 = c - a;

        packed_float3 n = normalize(cross(e1, e2));

        if (!all_finite(n)) {
            bad_triangles.push_back(i);
        }
    }

    if (!bad_triangles.empty()) {
        std::cout << "Found " << bad_triangles.size() << " degenerate triangles!" << std::endl;

        std::vector<Index_triangle> good_triangles(triangles.size() - bad_triangles.size());

        uint32_t const blen = uint32_t(bad_triangles.size());

        for (uint32_t i = 0, b = 0, g = 0, len = uint32_t(good_triangles.size()); g < len; ++i) {
            if (b >= blen || i != bad_triangles[b]) {
                good_triangles[g] = triangles[i];
                ++g;
            } else {
                ++b;
            }
        }

        std::swap(triangles, good_triangles);

        uint32_t remove_start = 0;

        uint32_t b = 0;

        for (auto& p : parts) {
            uint32_t const triangles_start = p.start_index / 3;
            uint32_t const triangles_end   = (p.start_index + p.num_indices) / 3;

            if (b < blen && triangles_start == bad_triangles[b]) {
                p.start_index -= 3;
            }

            uint32_t remove_num = 0;

            for (; b < blen; ++b) {
                if (triangles_end > bad_triangles[b]) {
                    remove_num += 3;
                } else {
                    break;
                }
            }

            p.start_index -= remove_start;
            p.num_indices -= remove_num;

            remove_start += remove_num;
        }
    }

    for (size_t i = 0, len = vertices.size(); i < len; ++i) {
        auto& v = vertices[i];

        if (float const sl = squared_length(v.n); sl < 0.9f || sl > 1.1f || !all_finite(v.n)) {
            //		std::cout << filename << " vertex " << i << std::endl;

            std::cout << "n: " << v.n << " | " << std::sqrt(sl) << " | converted to ";
            v.n = packed_float3(0.f, 1.f, 0.f);
            std::cout << v.n << std::endl;

            success = false;
        }

        if (float const sl = squared_length(v.t); sl < 0.9f || sl > 1.1f || !all_finite(v.t)) {
            //	std::cout << filename << " vertex " << i << std::endl;

            std::cout << "t: " << v.t << " | " << std::sqrt(sl) << " | converted to ";
            v.t = packed_float3(tangent(float3(v.n)));
            std::cout << v.t << std::endl;

            success = false;
        }
    }

    return success;
}
#endif

}  // namespace scene::shape::triangle
