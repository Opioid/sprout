#pragma once

#include "base/json/rapidjson_types.hpp"
#include <string>

namespace scene {

namespace surrounding {

class Surrounding;

}

class Scene;

class Loader {
public:

	bool load(const std::string& filename, Scene& scene);

private:

	surrounding::Surrounding* load_surrounding(const rapidjson::Value& surrounding_value) const;
};

}
