#pragma once

#include "base/json/rapidjson_types.hpp"
#include <string>
#include <memory>

namespace scene {

namespace surrounding {

class Surrounding;

}

namespace shape {

class Shape;

}

class Scene;
class Prop;

class Loader {
public:

	Loader();

	bool load(const std::string& filename, Scene& scene) const;

private:

	surrounding::Surrounding* load_surrounding(const rapidjson::Value& surrounding_value) const;

	void load_entities(const rapidjson::Value& entities_value, Scene& scene) const;

	Prop* load_prop(const rapidjson::Value& prop_value, Scene& scene) const;

	std::shared_ptr<shape::Shape> load_shape(const rapidjson::Value& shape_value) const;

	std::shared_ptr<shape::Shape> shape(const std::string& type) const;

	std::shared_ptr<shape::Shape> sphere_;
};

}
