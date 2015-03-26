#pragma once

#include "resource/resource_provider.hpp"
#include "scene/shape/geometry/vertex.hpp"
#include "base/json/rapidjson_types.hpp"
#include <string>
#include <memory>
#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;
class Mesh;

class Provider : public resource::Provider<Mesh> {
public:

	virtual std::shared_ptr<Mesh> load(const std::string& filename);

private:

	struct Group {
		uint32_t material_index;
		uint32_t start_index;
		uint32_t num_indices;
	};

	void load_mesh_data(const rapidjson::Value& geometry_value, std::vector<Index_triangle>& triangles, std::vector<Vertex>& vertices) const;

	void load_groups(const rapidjson::Value& groups_value, std::vector<Group>& groups) const;
};

}}}
