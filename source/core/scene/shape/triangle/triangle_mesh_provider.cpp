#include "triangle_mesh_provider.hpp"
#include "resource/resource_provider.inl"
#include "resource/resource_manager.hpp"
#include "triangle_type.hpp"
#include "triangle_json_handler.hpp"
#include "triangle_morphable_mesh.hpp"
#include "triangle_morph_target_collection.hpp"
#include "triangle_mesh.hpp"
#include "triangle_primitive.hpp"
#include "bvh/triangle_bvh_builder_sah.inl"
#include "bvh/triangle_bvh_builder_suh.inl"
#include "bvh/triangle_bvh_data.inl"
#include "bvh/triangle_bvh_data_interleaved.inl"
#include "bvh/triangle_bvh_indexed_data.inl"
#include "file/file_system.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"
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

	std::vector<Index_triangle> triangles;
	std::vector<Vertex> vertices;
	uint32_t num_parts = 0;

	{
		auto stream_pointer = manager.file_system().read_stream(filename);
		rapidjson::IStreamWrapper json_stream(*stream_pointer);

		Json_handler handler;

		rapidjson::Reader reader;

		reader.Parse(json_stream, handler);

		if (handler.parts().empty()) {
			handler.create_part();
		}

		if (!handler.morph_targets().empty()) {
			return load_morphable_mesh(filename, handler.morph_targets(), manager);
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

	SOFT_ASSERT(check(vertices, filename));

    return create_mesh(triangles, vertices, num_parts, bvh_preset, manager.thread_pool());
}

std::shared_ptr<Shape> Provider::load(const void* /*data*/,
									  const std::string& /*mount_folder*/,
									  const memory::Variant_map& /*options*/,
									  resource::Manager& /*manager*/) {
	return nullptr;
}

std::shared_ptr<Shape> Provider::create_mesh(const std::vector<Index_triangle>& triangles,
                                             const std::vector<Vertex>& vertices,
                                             uint32_t num_parts, BVH_preset bvh_preset,
                                             thread::Pool& thread_pool) {
	if (triangles.empty() || vertices.empty()) {
		throw std::runtime_error("No mesh data");
	}

    auto mesh = std::make_shared<Mesh>();

	if (BVH_preset::Fast == bvh_preset) {
		bvh::Builder_SUH builder;
		builder.build(mesh->tree(), triangles, vertices, num_parts, 8);
	} else {
		bvh::Builder_SAH builder(16, 64);
		builder.build(mesh->tree(), triangles, vertices, num_parts, 4, thread_pool);
	}

    if (!mesh->init()) {
        throw std::runtime_error("Mesh could not be initialized");
    }

    return mesh;
}

std::shared_ptr<Shape> Provider::load_morphable_mesh(const std::string& filename,
													 const std::vector<std::string>& morph_targets,
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

					collection->triangles().push_back(Index_triangle(a, b, c, p.material_index));
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

