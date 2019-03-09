#include "triangle_mesh_provider.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/distribution/distribution_1d.inl"
#include "base/math/vector3.inl"
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

Provider::Provider() noexcept : resource::Provider<Shape>("Mesh") {}

Provider::~Provider() noexcept {}

Shape* Provider::load(std::string const& filename, memory::Variant_map const& /*options*/,
                      resource::Manager& manager) {
    auto stream_pointer = manager.filesystem().read_stream(filename);
    if (!stream_pointer) {
        return nullptr;
    }

    file::Type type = file::query_type(*stream_pointer);
    if (file::Type::SUB == type) {
        return load_binary(*stream_pointer, manager.thread_pool());
    }

    Json_handler handler;

    {
        rapidjson::IStreamWrapper json_stream(*stream_pointer);

        rapidjson::Reader reader;

        reader.Parse(json_stream, handler);

        delete stream_pointer.release();
    }

    if (!handler.morph_targets().empty()) {
        return load_morphable_mesh(filename, handler.morph_targets(), manager);
    }

    if (handler.vertices().empty()) {
        logging::error("Mesh \"" + filename + "\" does not contain vertices");
        return nullptr;
    }

    if (!handler.has_positions()) {
        logging::error("Mesh \"" + filename + "\" does not contain vertex positions");
        return nullptr;
    }

    if (handler.triangles().empty()) {
        logging::error("Mesh \"" + filename + "\" does not contain indices");
        return nullptr;
    }

    if (handler.parts().empty()) {
        handler.create_part();
    }

    if (!handler.has_normals()) {
        // If no normals were loaded, assign something.
        // Might be smarter to throw an exception,
        // or just go ahead and actually compute the geometry normal...
        for (auto& v : handler.vertices()) {
            v.n = packed_float3(0.f, 1.f, 0.f);
        }
    }

    if (!handler.has_tangents()) {
        // If no tangents were loaded, compute the tangent space manually
        for (auto& v : handler.vertices()) {
            packed_float3 b;
            orthonormal_basis(v.n, v.t, b);
            v.bitangent_sign = 1.f;
        }
    }

    SOFT_ASSERT(check_and_fix(handler.vertices(), filename));

    // Exporter::write(filename, handler);

    auto mesh = new Mesh;

    mesh->tree().allocate_parts(static_cast<uint32_t>(handler.parts().size()));

    manager.thread_pool().run_async([mesh, parts = std::move(handler.parts()),
                                     triangles = std::move(handler.triangles()),
                                     vertices = std::move(handler.vertices()), &manager]() mutable {
        logging::verbose("Started asynchronously building triangle mesh BVH.");

        for (auto& p : parts) {
            uint32_t const triangles_start = p.start_index / 3;
            uint32_t const triangles_end   = (p.start_index + p.num_indices) / 3;

            for (uint32_t i = triangles_start; i < triangles_end; ++i) {
                triangles[i].material_index = p.material_index;
            }
        }

        build_bvh(*mesh, triangles, static_cast<uint32_t>(vertices.size()), vertices.data(),
                  manager.thread_pool());

        logging::verbose("Finished asynchronously building triangle mesh BVH.");
    });

    return mesh;
}

Shape* Provider::load(void const* /*data*/, std::string_view /*mount_folder*/,
                      memory::Variant_map const& /*options*/, resource::Manager& /*manager*/) {
    return nullptr;
}

size_t Provider::num_bytes() const noexcept {
    return sizeof(*this);
}

Shape* Provider::create_mesh(Triangles const& triangles, Vertices const& vertices,
                             uint32_t num_parts, thread::Pool& thread_pool) {
    if (triangles.empty() || vertices.empty() || !num_parts) {
        logging::error("No mesh data.");
        return nullptr;
    }

    auto mesh = new Mesh;

    mesh->tree().allocate_parts(num_parts);

    thread_pool.run_async([mesh, triangles_in = std::move(triangles),
                           vertices_in = std::move(vertices), &thread_pool]() {
        build_bvh(*mesh, triangles_in, static_cast<uint32_t>(vertices_in.size()),
                  vertices_in.data(), thread_pool);
    });

    //	build_bvh(*mesh, triangles, vertices, bvh_preset, thread_pool);

    return mesh;
}

Shape* Provider::load_morphable_mesh(std::string const& filename, Strings const& morph_targets,
                                     resource::Manager& manager) {
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
                v.bitangent_sign = 1.f;
            }
        }

        // The idea is to have one identical set of indices for all morph targets
        if (collection->triangles().empty()) {
            auto& triangles = handler.triangles();

            for (auto& p : handler.parts()) {
                uint32_t triangles_start = p.start_index / 3;
                uint32_t triangles_end   = (p.start_index + p.num_indices) / 3;

                for (uint32_t i = triangles_start; i < triangles_end; ++i) {
                    uint32_t a = triangles[i].i[0];
                    uint32_t b = triangles[i].i[1];
                    uint32_t c = triangles[i].i[2];

                    collection->triangles().emplace_back(a, b, c, p.material_index);
                }
            }
        }

        (void)filename;
        SOFT_ASSERT(check(handler.vertices(), filename));

        collection->add_swap_vertices(handler.vertices());
    }

    if (collection->triangles().empty()) {
        return nullptr;
    }

    uint32_t const num_parts = static_cast<uint32_t>(handler.parts().size());

    auto mesh = new Morphable_mesh(collection, num_parts);

    return mesh;
}

void Provider::build_bvh(Mesh& mesh, Triangles const& triangles, uint32_t num_vertices,
                         Vertex const* const vertices, thread::Pool& thread_pool) {
    bvh::Builder_SAH builder(16, 64);
    builder.build(mesh.tree(), triangles, num_vertices, vertices, 4, thread_pool);

    mesh.init();
}

template <typename Index>
void fill_triangles(const std::vector<Part>& parts, Index const* indices,
                    std::vector<Index_triangle>& triangles) {
    for (auto const& p : parts) {
        uint32_t const triangles_start = p.start_index / 3;
        uint32_t const triangles_end   = (p.start_index + p.num_indices) / 3;

        for (uint32_t i = triangles_start; i < triangles_end; ++i) {
            triangles[i].i[0] = static_cast<uint32_t>(indices[i * 3 + 0]);
            triangles[i].i[1] = static_cast<uint32_t>(indices[i * 3 + 1]);
            triangles[i].i[2] = static_cast<uint32_t>(indices[i * 3 + 2]);

            triangles[i].material_index = p.material_index;
        }
    }
}

Shape* Provider::load_binary(std::istream& stream, thread::Pool& thread_pool) {
    stream.seekg(4);

    uint64_t json_size = 0;
    stream.read(reinterpret_cast<char*>(&json_size), sizeof(uint64_t));

    char* json_string = new char[json_size + 1];
    stream.read(json_string, static_cast<std::streamsize>(json_size * sizeof(char)));
    json_string[json_size] = 0;

    std::string error;
    auto const  root = json::parse_insitu(json_string, error);
    if (!root) {
        logging::error("Shape: " + error);
        return nullptr;
    }

    json::Value::ConstMemberIterator const geometry_node = root->FindMember("geometry");
    if (root->MemberEnd() == geometry_node) {
        delete[] json_string;
        logging::error("Model has no geometry node.");
        return nullptr;
    }

    json::Value const& geometry_value = geometry_node->value;

    std::vector<Part> parts;

    uint64_t vertices_offset = 0;
    uint64_t vertices_size   = 0;

    uint64_t indices_offset = 0;
    uint64_t indices_size   = 0;
    uint64_t index_bytes    = 0;

    for (auto& n : geometry_value.GetObject()) {
        if ("parts" == n.name) {
            for (auto const& pn : n.value.GetArray()) {
                parts.emplace_back(json::read_uint(pn, "start_index"),
                                   json::read_uint(pn, "num_indices"),
                                   json::read_uint(pn, "material_index"));
            }
        } else if ("vertices" == n.name) {
            for (auto const& vn : n.value.GetObject()) {
                if ("binary" == vn.name) {
                    vertices_offset = json::read_uint(vn.value, "offset");
                    vertices_size   = json::read_uint(vn.value, "size");
                }
            }
        } else if ("indices" == n.name) {
            for (auto const& in : n.value.GetObject()) {
                if ("binary" == in.name) {
                    indices_offset = json::read_uint(in.value, "offset");
                    indices_size   = json::read_uint(in.value, "size");
                } else if ("encoding" == in.name) {
                    if ("UInt16" == json::read_string(in.value)) {
                        index_bytes = 2;
                    } else {
                        index_bytes = 4;
                    }
                }
            }
        }
    }

    delete[] json_string;

    uint64_t const binary_start = json_size + 4u + sizeof(uint64_t);

    std::vector<Vertex> vertices(vertices_size / sizeof(Vertex));

    stream.seekg(static_cast<std::streamoff>(binary_start + vertices_offset));
    stream.read(reinterpret_cast<char*>(vertices.data()),
                static_cast<std::streamsize>(vertices_size));

    uint64_t const num_indices = indices_size / index_bytes;

    char* indices = new char[indices_size];

    stream.seekg(static_cast<std::streamoff>(binary_start + indices_offset));
    stream.read(indices, static_cast<std::streamsize>(indices_size));

    auto mesh = new Mesh;

    mesh->tree().allocate_parts(static_cast<uint32_t>(parts.size()));

    thread_pool.run_async([mesh, local_parts = std::move(parts), local_indices = std::move(indices),
                           num_indices, local_vertices = std::move(vertices), index_bytes,
                           &thread_pool]() {
        std::vector<Index_triangle> triangles(num_indices / 3);

        if (4 == index_bytes) {
            uint32_t* indices32 = reinterpret_cast<uint32_t*>(local_indices);
            fill_triangles(local_parts, indices32, triangles);
        } else {
            uint16_t* indices16 = reinterpret_cast<uint16_t*>(local_indices);
            fill_triangles(local_parts, indices16, triangles);
        }

        delete[] local_indices;

        build_bvh(*mesh, triangles, static_cast<uint32_t>(local_vertices.size()),
                  local_vertices.data(), thread_pool);
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
