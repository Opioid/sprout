#ifndef SU_EXTENSION_PROCEDURAL_MESH_GRASS_HPP
#define SU_EXTENSION_PROCEDURAL_MESH_GRASS_HPP

#include "core/scene/shape/triangle/triangle_mesh_generator.hpp"
#include "base/math/vector3.hpp"
#include <vector>

namespace scene::shape {

namespace triangle { struct Index_triangle; }

struct Vertex;

}

namespace procedural::mesh {

class Grass : public scene::shape::triangle::Generator {

public:

	using Shape = scene::shape::Shape;

	virtual std::shared_ptr<scene::shape::Shape> create_mesh(
            const json::Value& mesh_value, resource::Manager& manager) override final;

private:

	void add_blade(const float3& offset,
				   float rotation_y, float lean_factor, float width, float height,
				   uint32_t vertex_offset,
				   std::vector<scene::shape::triangle::Index_triangle>& triangles,
				   std::vector<scene::shape::Vertex>& vertices) const;

	static void calculate_normals(std::vector<scene::shape::triangle::Index_triangle>& triangles,
								  std::vector<scene::shape::Vertex>& vertices);
};

}

#endif
