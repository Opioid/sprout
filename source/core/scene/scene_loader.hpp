#pragma once

#include <string>

namespace scene {

class Scene;

class Loader {
public:

	bool load(const std::string& filename, Scene& scene);
};

}
