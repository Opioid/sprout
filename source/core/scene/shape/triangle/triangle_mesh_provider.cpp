#include "triangle_mesh_provider.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_manager.hpp"
#include "triangle_type.hpp"
#include "triangle_json_handler.hpp"
#include "triangle_morphable_mesh.hpp"
#include "triangle_morph_target_collection.hpp"
#include "triangle_mesh.hpp"
#include "triangle_primitive.hpp"

#include "triangle_mesh_exporter.hpp"

// #include "bvh/triangle_bvh_builder_sah.inl"
#include "bvh/triangle_bvh_builder_sah2.inl"
#include "bvh/triangle_bvh_builder_suh.inl"
#include "bvh/triangle_bvh_data.inl"
#include "bvh/triangle_bvh_data_interleaved.inl"
#include "bvh/triangle_bvh_indexed_data.inl"
#include "file/file.hpp"
#include "file/file_system.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/variant_map.inl"
#include "rapidjson/istreamwrapper.h"

#include "base/debug/assert.hpp"
#ifdef SU_DEBUG
#	include "base/math/print.hpp"
#	include <iostream>
#endif

namespace scene { namespace shape { namespace triangle {

#ifdef SU_DEBUG
bool check(const std::vector<Vertex>& vertices, const std::string& filename);
#endif

Provider::Provider() : resource::Provider<Shape>("Mesh") {}

Provider::~Provider() {}

std::shared_ptr<Shape> Provider::load(const std::string& filename,
									  const memory::Variant_map& options,
									  resource::Manager& manager) {
	BVH_preset bvh_preset = BVH_preset::Unknown;
	options.query("bvh_preset", bvh_preset);

	auto stream_pointer = manager.file_system().read_stream(filename);

	file::Type type = file::query_type(*stream_pointer);
	if (file::Type::SUM == type) {
		return load_binary(*stream_pointer, manager.thread_pool());
	}

	rapidjson::IStreamWrapper json_stream(*stream_pointer);

	Json_handler handler;

	rapidjson::Reader reader;

	reader.Parse(json_stream, handler);

	if (!handler.morph_targets().empty()) {
		return load_morphable_mesh(filename, handler.morph_targets(), manager);
	}

	if (handler.vertices().empty()) {
		throw std::runtime_error("Mesh does not contain vertices");
	}

	if (!handler.has_positions()) {
		throw std::runtime_error("Mesh does not contain vertex positions");
	}

	if (handler.triangles().empty()) {
		throw std::runtime_error("Mesh does not contain indices");
	}

	if (handler.parts().empty()) {
		handler.create_part();
	}

	if (!handler.has_normals()) {
		// If no normals were loaded assign something.
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
			math::orthonormal_basis(v.n, v.t, b);
			v.bitangent_sign = 1.f;
		}
	}

	if (BVH_preset::Unknown == bvh_preset) {
		bvh_preset = handler.bvh_preset();
	}

	SOFT_ASSERT(check(handler.vertices(), filename));

//	Exporter::write(filename, handler);

	auto mesh = std::make_shared<Mesh>();

	mesh->tree().allocate_parts(static_cast<uint32_t>(handler.parts().size()));

	manager.thread_pool().run_async(
		[mesh, parts = std::move(handler.parts()), triangles = std::move(handler.triangles()),
		 vertices = std::move(handler.vertices()), bvh_preset, &manager]() mutable {
			for (auto& p : parts) {
				uint32_t triangles_start = p.start_index / 3;
				uint32_t triangles_end = (p.start_index + p.num_indices) / 3;

				for (uint32_t i = triangles_start; i < triangles_end; ++i) {
					triangles[i].material_index = p.material_index;
				}
			}

			build_bvh(*mesh, triangles, vertices, bvh_preset, manager.thread_pool());
		}
	);

	return mesh;
}

std::shared_ptr<Shape> Provider::load(const void* /*data*/,
									  const std::string& /*mount_folder*/,
									  const memory::Variant_map& /*options*/,
									  resource::Manager& /*manager*/) {
	return nullptr;
}

size_t Provider::num_bytes() const {
	return sizeof(*this);
}

std::shared_ptr<Shape> Provider::create_mesh(const Triangles& triangles, const Vertices& vertices,
                                             uint32_t num_parts, BVH_preset bvh_preset,
                                             thread::Pool& thread_pool) {
	if (triangles.empty() || vertices.empty() || !num_parts) {
		throw std::runtime_error("No mesh data");
	}

    auto mesh = std::make_shared<Mesh>();

	mesh->tree().allocate_parts(num_parts);

	build_bvh(*mesh, triangles, vertices, bvh_preset, thread_pool);

    return mesh;
}

std::shared_ptr<Shape> Provider::load_morphable_mesh(const std::string& filename,
													 const Strings& morph_targets,
													 resource::Manager& manager) {
	auto collection = std::make_shared<Morph_target_collection>();

	Json_handler handler;

	for (auto& targets : morph_targets) {
		auto stream_pointer = manager.file_system().read_stream(targets);

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
				math::orthonormal_basis(v.n, v.t, b);
				v.bitangent_sign = 1.f;
			}
		}

		// The idea is to have one identical set of indices for all morph targets
		if (collection->triangles().empty()) {
			auto& triangles = handler.triangles();

			for (auto& p : handler.parts()) {
				uint32_t triangles_start = p.start_index / 3;
				uint32_t triangles_end = (p.start_index + p.num_indices) / 3;

				for (uint32_t i = triangles_start; i < triangles_end; ++i) {
					uint32_t a = triangles[i].i[0];
					uint32_t b = triangles[i].i[1];
					uint32_t c = triangles[i].i[2];

					collection->triangles().emplace_back(a, b, c, p.material_index);
				}
			}
		}

		SOFT_ASSERT(check(handler.vertices(), filename));

		collection->add_swap_vertices(handler.vertices());
	}

	if (collection->triangles().empty()) {
		return nullptr;
	}

	uint32_t num_parts = static_cast<uint32_t>(handler.parts().size());
	auto mesh = std::make_shared<Morphable_mesh>(collection, num_parts);

	return mesh;
}

void Provider::build_bvh(Mesh& mesh, const Triangles& triangles, const Vertices& vertices,
						 BVH_preset bvh_preset, thread::Pool& thread_pool) {
	if (BVH_preset::Fast == bvh_preset) {
		bvh::Builder_SUH builder;
		builder.build(mesh.tree(), triangles, vertices, 8);
	} else {
//		bvh::Builder_SAH builder(16, 64);
//		builder.build(mesh->tree(), triangles, vertices, 4, thread_pool);

		bvh::Builder_SAH2 builder(16, 64);
		builder.build(mesh.tree(), triangles, vertices, 4, thread_pool);
	}

	mesh.init();
}

template<typename Index>
void fill_triangles(const std::vector<Part>& parts, const Index* indices,
					std::vector<Index_triangle>& triangles) {
	for (auto& p : parts) {
		uint32_t triangles_start = p.start_index / 3;
		uint32_t triangles_end = (p.start_index + p.num_indices) / 3;

		for (uint32_t i = triangles_start; i < triangles_end; ++i) {
			triangles[i].i[0] = static_cast<uint32_t>(indices[i * 3 + 0]);
			triangles[i].i[1] = static_cast<uint32_t>(indices[i * 3 + 1]);
			triangles[i].i[2] = static_cast<uint32_t>(indices[i * 3 + 2]);

			triangles[i].material_index = p.material_index;
		}
	}
}

std::shared_ptr<Shape> Provider::load_binary(std::istream& stream, thread::Pool& thread_pool) {
	std::cout << "Load binary" << std::endl;

	stream.seekg(4);

	uint64_t json_size = 0;
	stream.read(reinterpret_cast<char*>(&json_size), sizeof(uint64_t));

	char* json_string = new char[json_size + 1];
	stream.read(json_string, json_size * sizeof(char));
	json_string[json_size] = 0;

	auto root = json::parse_insitu(json_string);

	const json::Value::ConstMemberIterator geometry_node = root->FindMember("geometry");
	if (root->MemberEnd() == geometry_node) {
		delete[] json_string;
		throw std::runtime_error("Model has no geometry node");
	}

	const json::Value& geometry_value = geometry_node->value;

	std::vector<Part> parts;

	size_t vertices_offset = 0;
	size_t vertices_size = 0;

	size_t indices_offset = 0;
	size_t indices_size = 0;
	size_t index_bytes = 0;

	for (auto& n : geometry_value.GetObject()) {
		if ("parts" == n.name) {
			for (auto& pn : n.value.GetArray()) {
				parts.emplace_back(json::read_uint(pn, "start_index"),
								   json::read_uint(pn, "num_indices"),
								   json::read_uint(pn, "material_index"));
			}
		} else if ("vertices" == n.name) {
			for (auto& vn : n.value.GetObject()) {
				if ("binary" == vn.name) {
					vertices_offset = json::read_uint(vn.value, "offset");
					vertices_size   = json::read_uint(vn.value, "size");
				}
			}
		} else if ("indices" == n.name) {
			for (auto& in : n.value.GetObject()) {
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

	int64_t binary_start = json_size + 4 + sizeof(uint64_t);

	std::vector<Vertex> vertices(vertices_size / sizeof(Vertex));

	stream.seekg(binary_start + vertices_offset);
	stream.read(reinterpret_cast<char*>(vertices.data()), vertices_size);

	size_t num_indices = indices_size / index_bytes;
	char* indices = new char[indices_size];

	stream.seekg(binary_start + indices_offset);
	stream.read(indices, indices_size);

	BVH_preset bvh_preset = BVH_preset::Slow;

	auto mesh = std::make_shared<Mesh>();

	mesh->tree().allocate_parts(static_cast<uint32_t>(parts.size()));

	thread_pool.run_async(
		[mesh, local_parts = std::move(parts), local_indices = std::move(indices), num_indices,
		 local_vertices = std::move(vertices), bvh_preset, index_bytes, &thread_pool]() {
			std::vector<Index_triangle> triangles(num_indices / 3);

			if (4 == index_bytes) {
				uint32_t* indices32 = reinterpret_cast<uint32_t*>(local_indices);
				fill_triangles(local_parts, indices32, triangles);
			} else {
				uint16_t* indices16 = reinterpret_cast<uint16_t*>(local_indices);
				fill_triangles(local_parts, indices16, triangles);
			}

			delete[] local_indices;

			build_bvh(*mesh, triangles, local_vertices, bvh_preset, thread_pool);
		}
	);

	return mesh;
}

#ifdef SU_DEBUG
bool check(const std::vector<Vertex>& vertices, const std::string& filename) {
	for (size_t i = 0, len = vertices.size(); i < len; ++i) {
		const auto& v = vertices[i];
		if (math::squared_length(v.n) < 0.1f ||
			math::squared_length(v.t) < 0.1f) {

			std::cout << filename << " vertex " << i << std::endl;

			std::cout << "n: " << v.n << std::endl;
			std::cout << "t: " << v.t << std::endl;

			return false;
		}
	}

	return true;
}
#endif

}}}

