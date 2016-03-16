#pragma once

#include "resource/resource_provider.hpp"
#include "triangle_bvh_preset.hpp"
#include <string>
#include <vector>

namespace scene { namespace shape {

class Shape;
struct Vertex;

namespace triangle {

struct Index_triangle;

class Provider : public resource::Provider<Shape> {
public:

	Provider();

	virtual std::shared_ptr<Shape> load(const std::string& filename,
										const memory::Variant_map& options,
										resource::Manager& manager) final override;

    std::shared_ptr<Shape> create_mesh(const std::vector<Index_triangle>& triangles,
                                       const std::vector<Vertex>& vertices,
                                       uint32_t num_parts, BVH_preset bvh_preset,
                                       thread::Pool& thread_pool);

private:

	std::shared_ptr<Shape> load_morphable_mesh(const std::string& filename,
											   const std::vector<std::string>& morph_targets,
											   resource::Manager& manager);
};

}}}
