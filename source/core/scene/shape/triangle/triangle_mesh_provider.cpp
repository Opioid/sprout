#include "triangle_mesh_provider.hpp"
#include "resource/resource_provider.inl"
#include "triangle_bvh_preset.hpp"
#include "triangle_type.hpp"
#include "triangle_json_handler.hpp"
#include "triangle_morphable_mesh.hpp"
#include "triangle_morph_target_collection.hpp"
#include "triangle_mesh.hpp"
#include "triangle_primitive.hpp"
#include "bvh/triangle_bvh_builder_sah.inl"
#include "bvh/triangle_bvh_builder_suh.inl"
#include "bvh/triangle_bvh_data_generic.inl"
#include "file/file_system.hpp"
#include "base/json/json.hpp"
#include "base/json/json_read_stream.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"
#include "base/memory/variant_map.inl"

namespace scene { namespace shape { namespace triangle {

Provider::Provider(file::System& file_system, thread::Pool& thread_pool) :
	resource::Provider<Shape>(file_system, thread_pool) {}

std::shared_ptr<Shape> Provider::load(const std::string& filename, const memory::Variant_map& options) {
	auto stream_pointer = file_system_.read_stream(filename);

	BVH_preset bvh_preset = BVH_preset::Unknown;
	options.query("bvh_preset", bvh_preset);

	std::vector<Index_triangle> triangles;
	std::vector<Vertex> vertices;

	{
		json::Read_stream json_stream(*stream_pointer);

		Json_handler handler;

		rapidjson::Reader reader;

		reader.Parse(json_stream, handler);

		if (handler.parts().empty()) {
			handler.create_part();
		}

		if (!handler.morph_targets().empty()) {
			return load_morphable_mesh(filename, handler.morph_targets());
		}

		if (!handler.has_positions()) {
			throw std::runtime_error("Mesh does not contain vertex positions");
		}

		if (!handler.has_normals()) {
			// If no normals were loaded assign identity
			// Might be smarter to throw an exception
			for (auto& v : handler.vertices()) {
				v.n = math::float3::identity;
			}
		}

		if (!handler.has_tangents()) {
			// If no tangents were loaded, compute the tangent space manually
			for (auto& v : handler.vertices()) {
				math::float3 b;
				math::coordinate_system(v.n, v.t, b);
				v.bitangent_sign = 1.f;
			}
		}

		auto& indices = handler.indices();

		for (auto& p : handler.parts()) {
			uint32_t triangles_start = p.start_index / 3;
			uint32_t triangles_end = (p.start_index + p.num_indices) / 3;

			for (uint32_t i = triangles_start; i < triangles_end; ++i) {
				uint32_t a = indices[i * 3 + 0];
				uint32_t b = indices[i * 3 + 1];
				uint32_t c = indices[i * 3 + 2];

				triangles.push_back(Index_triangle{a, b, c, p.material_index});
			}
		}

		vertices.swap(handler.vertices());

		if (BVH_preset::Unknown == bvh_preset) {
			bvh_preset = handler.bvh_preset();
		}
	}

	auto mesh = std::make_shared<Mesh>();

	if (BVH_preset::Slow == bvh_preset) {
		bvh::Builder_SAH builder(16, 64);
		builder.build<bvh::Data_generic<Triangle_type>>(mesh->tree_, triangles, vertices, 4, thread_pool_);
	} else {
		bvh::Builder_SUH builder;
		builder.build<bvh::Data_generic<Triangle_type>>(mesh->tree_, triangles, vertices, 8);
	}

	mesh->init();

	return mesh;
}

std::shared_ptr<Shape> Provider::load_morphable_mesh(const std::string& /*filename*/,
													 const std::vector<std::string>& morph_targets) {
	auto collection = std::make_shared<Morph_target_collection>();

	Json_handler handler;

	for (auto& targets : morph_targets) {
		auto stream_pointer = file_system_.read_stream(targets);

		json::Read_stream json_stream(*stream_pointer);

		handler.clear();

		rapidjson::Reader reader;

		reader.Parse(json_stream, handler);

		if (!handler.has_positions()) {
			continue;
		}

		if (!handler.has_normals()) {
			// If no normals were loaded assign identity
			// Might be smarter to throw an exception
			for (auto& v : handler.vertices()) {
				v.n = math::float3::identity;
			}
		}

		if (!handler.has_tangents()) {
			// If no tangents were loaded, compute the tangent space manually
			for (auto& v : handler.vertices()) {
				math::float3 b;
				math::coordinate_system(v.n, v.t, b);
				v.bitangent_sign = 1.f;
			}
		}

		// The idea is to have one identical set of indices for all morph targets
		if (collection->triangles().empty()) {
			auto& indices = handler.indices();

			for (auto& p : handler.parts()) {
				uint32_t triangles_start = p.start_index / 3;
				uint32_t triangles_end = (p.start_index + p.num_indices) / 3;

				for (uint32_t i = triangles_start; i < triangles_end; ++i) {
					uint32_t a = indices[i * 3 + 0];
					uint32_t b = indices[i * 3 + 1];
					uint32_t c = indices[i * 3 + 2];

					collection->triangles().push_back(Index_triangle{a, b, c, p.material_index});
				}
			}
		}

		collection->add_swap_vertices(handler.vertices());
	}

	if (collection->triangles().empty()) {
		return nullptr;
	}

	auto mesh = std::make_shared<Morphable_mesh>(collection, static_cast<uint32_t>(handler.parts().size()));

	return mesh;
}

}}}

