#pragma once

#include "core/scene/shape/triangle/triangle_mesh_generator.hpp"
#include "base/math/vector.hpp"
#include "base/math/matrix.hpp"
#include <vector>

namespace scene { namespace shape {

namespace triangle { struct Index_triangle; }

struct Vertex;

}}

namespace procedural { namespace mesh {

class Grass : public scene::shape::triangle::Generator {
public:

	virtual std::shared_ptr<scene::shape::Shape> create_mesh(
			const rapidjson::Value& mesh_value, resource::Manager& manager) final override;

private:

	void add_blade(const math::packed_float3& offset,
				   float rotation_y, float lean_factor, float width, float height,
				   uint32_t vertex_offset,
				   std::vector<scene::shape::triangle::Index_triangle>& triangles,
				   std::vector<scene::shape::Vertex>& vertices) const;

	static void calculate_normals(std::vector<scene::shape::triangle::Index_triangle>& triangles,
								  std::vector<scene::shape::Vertex>& vertices);
};

}}

