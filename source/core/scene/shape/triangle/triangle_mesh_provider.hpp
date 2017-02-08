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

class Provider : public resource::Provider<Shape> {

public:

	using Triangles = std::vector<Index_triangle>;
	using Vertices  = std::vector<Vertex>;

	Provider();
	~Provider();

	virtual std::shared_ptr<Shape> load(const std::string& filename,
										const memory::Variant_map& options,
										resource::Manager& manager) final override;

	virtual std::shared_ptr<Shape> load(const void* data,
										const std::string& mount_folder,
										const memory::Variant_map& options,
										resource::Manager& manager) final override;

	virtual size_t num_bytes() const final override;

	static std::shared_ptr<Shape> create_mesh(const Triangles& triangles,
											  const Vertices& vertices,
											  uint32_t num_parts, BVH_preset bvh_preset,
											  thread::Pool& thread_pool);

private:

	std::shared_ptr<Shape> load_morphable_mesh(const std::string& filename,
											   const std::vector<std::string>& morph_targets,
											   resource::Manager& manager);
};

}}}
