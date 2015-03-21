#pragma once

#include "base/json/rapidjson_types.hpp"
#include "shape/triangle/triangle_mesh_provider.hpp"
#include "resource/resource_cache.hpp"
#include <string>
#include <memory>

namespace scene {

namespace surrounding {

class Surrounding;

}

namespace shape {

class Shape;

namespace triangle {

class Mesh;

}}

class Scene;
class Prop;

class Loader {
public:

	Loader();

	bool load(const std::string& filename, Scene& scene);

private:

	surrounding::Surrounding* load_surrounding(const rapidjson::Value& surrounding_value) const;

	void load_entities(const rapidjson::Value& entities_value, Scene& scene);

	Prop* load_prop(const rapidjson::Value& prop_value, Scene& scene);

	std::shared_ptr<shape::Shape> load_shape(const rapidjson::Value& shape_value);

	std::shared_ptr<shape::Shape> shape(const std::string& type) const;

	std::shared_ptr<shape::Shape> plane_;
	std::shared_ptr<shape::Shape> sphere_;

	resource::Cache<shape::triangle::Mesh> mesh_cache_;
	shape::triangle::Provider mesh_provider_;
};

}
