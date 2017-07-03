#pragma once

#include "resource/resource_provider.hpp"
#include "triangle_bvh_preset.hpp"
#include <string>
#include <vector>

namespace thread { class Pool; }

namespace scene { namespace shape {

class Shape;
struct Vertex;

namespace triangle {

struct Index_triangle;
class Mesh;

class Provider : public resource::Provider<Shape> {

public:

	using Triangles = std::vector<Index_triangle>;
	using Vertices  = std::vector<Vertex>;
	using Strings   = std::vector<std::string>;

	Provider();
	~Provider();

	virtual std::shared_ptr<Shape> load(const std::string& filename,
										const memory::Variant_map& options,
										resource::Manager& manager) override final;

	virtual std::shared_ptr<Shape> load(const void* data,
										const std::string& mount_folder,
										const memory::Variant_map& options,
										resource::Manager& manager) override final;

	virtual size_t num_bytes() const override final;

	static std::shared_ptr<Shape> create_mesh(const Triangles& triangles, const Vertices& vertices,
											  uint32_t num_parts, BVH_preset bvh_preset,
											  thread::Pool& thread_pool);

private:

	std::shared_ptr<Shape> load_morphable_mesh(const std::string& filename,
											   const Strings& morph_targets,
											   resource::Manager& manager);

	static void build_bvh(Mesh& mesh, const Triangles& triangles, const Vertices& vertices,
						  BVH_preset bvh_preset, thread::Pool& thread_pool);

//	static void build_bvh(Mesh& mesh, const Triangles& triangles, const Vertices& vertices,
//						  BVH_preset bvh_preset, thread::Pool& thread_pool);

	static std::shared_ptr<Shape> load_binary(std::istream& stream, thread::Pool& thread_pool);
};

}}}
