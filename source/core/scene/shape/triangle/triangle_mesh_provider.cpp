#include "triangle_mesh_provider.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_manager.hpp"
#include "triangle_type.hpp"
#include "triangle_json_handler.hpp"
#include "triangle_morphable_mesh.hpp"
#include "triangle_morph_target_collection.hpp"
#include "triangle_mesh.hpp"
#include "triangle_primitive.hpp"
// #include "bvh/triangle_bvh_builder_sah.inl"
#include "bvh/triangle_bvh_builder_sah2.inl"
#include "bvh/triangle_bvh_builder_suh.inl"
#include "bvh/triangle_bvh_data.inl"
#include "bvh/triangle_bvh_data_interleaved.inl"
#include "bvh/triangle_bvh_indexed_data.inl"
#include "file/file_system.hpp"
#include "base/json/json.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector.inl"
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

	Triangles triangles;
	Vertices vertices;
	uint32_t num_parts = 0;

	{
		auto stream_pointer = manager.file_system().read_stream(filename);
		rapidjson::IStreamWrapper json_stream(*stream_pointer);

		Json_handler handler;

		rapidjson::Reader reader;

		reader.Parse(json_stream, handler);

		if (!handler.morph_targets().empty()) {
			return load_morphable_mesh(filename, handler.morph_targets(), manager);
		}

		if (handler.parts().empty()) {
			handler.create_part();
		}

		if (!handler.has_positions()) {
			throw std::runtime_error("Mesh does not contain vertex positions");
		}

		if (handler.triangles().empty()) {
			throw std::runtime_error("Mesh does not contain indices");
		}

		if (!handler.has_normals()) {
			// If no normals were loaded assign something.
			// Might be smarter to throw an exception,
			// or just go ahead and actually compute the geometry normal...
			for (auto& v : handler.vertices()) {
				v.n = math::packed_float3(0.f, 1.f, 0.f);
			}
		}

		if (!handler.has_tangents()) {
			// If no tangents were loaded, compute the tangent space manually
			for (auto& v : handler.vertices()) {
				math::packed_float3 b;
				math::coordinate_system(v.n, v.t, b);
				v.bitangent_sign = 1.f;
			}
		}

		triangles.swap(handler.triangles());

		for (auto& p : handler.parts()) {
			uint32_t triangles_start = p.start_index / 3;
			uint32_t triangles_end = (p.start_index + p.num_indices) / 3;

			for (uint32_t i = triangles_start; i < triangles_end; ++i) {
				triangles[i].material_index = p.material_index;
			}
		}

		vertices.swap(handler.vertices());

		num_parts = static_cast<uint32_t>(handler.parts().size());

		if (BVH_preset::Unknown == bvh_preset) {
			bvh_preset = handler.bvh_preset();
		}
	}

	if (triangles.empty() || vertices.empty() || !num_parts) {
		throw std::runtime_error("No mesh data");
	}

	SOFT_ASSERT(check(vertices, filename));

	auto mesh = std::make_shared<Mesh>();

	mesh->tree().allocate_parts(num_parts);

	manager.thread_pool().run_async(
		[mesh, captured_triangles = std::move(triangles),
		 captured_vertices = std::move(vertices), bvh_preset, &manager]() {
			build_bvh(*mesh, captured_triangles, captured_vertices,
					  bvh_preset, manager.thread_pool());
		}
	);

//	build_bvh(*mesh, triangles, vertices, bvh_preset, manager.thread_pool());

 //   return create_mesh(triangles, vertices, num_parts, bvh_preset, manager.thread_pool());

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
				v.n = math::packed_float3(0.f, 0.f, 1.f);
			}
		}

		if (!handler.has_tangents()) {
			// If no tangents were loaded, compute the tangent space manually
			for (auto& v : handler.vertices()) {
				math::packed_float3 b;
				math::coordinate_system(v.n, v.t, b);
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
					uint32_t a = triangles[i].a;
					uint32_t b = triangles[i].b;
					uint32_t c = triangles[i].c;

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

