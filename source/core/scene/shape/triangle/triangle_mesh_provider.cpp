#include "triangle_mesh_provider.hpp"
#include "resource/resource_provider.inl"
#include "triangle_json_handler.hpp"
#include "triangle_mesh.hpp"
#include "triangle_primitive.hpp"
#include "bvh/triangle_bvh_builder.inl"
#include "bvh/triangle_bvh_data_mt.inl"
#include "bvh/triangle_bvh_data_yf.inl"
#include "file/file_system.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"
#include "base/json/json.hpp"
#include "base/json/json_read_stream.hpp"
#include <iostream>

namespace scene { namespace shape { namespace triangle {

Provider::Provider(file::System& file_system) : resource::Provider<Mesh>(file_system) {}

std::shared_ptr<Mesh> Provider::load(const std::string& filename, uint32_t /*flags*/) {
	auto stream_pointer = file_system_.read_stream(filename);
	if (!*stream_pointer) {
		throw std::runtime_error("Stream could not be opened");
	}

	std::vector<Index_triangle> triangles;
	std::vector<Vertex> vertices;

	{
		json::Read_stream json_stream(*stream_pointer);

		Json_handler handler;

		rapidjson::Reader reader;

		reader.Parse(json_stream, handler);

		if (!handler.has_positions()) {
			throw std::runtime_error("Model does not contain vertex positions");
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
			}
		}

		auto& indices = handler.indices();

		for (auto& p : handler.groups()) {
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
	}

	auto mesh = std::make_shared<Mesh>();

	bvh::Builder builder;
    builder.build<bvh::Data_MT>(mesh->tree_, triangles, vertices, 8);

	mesh->init();

	return mesh;
}

}}}

