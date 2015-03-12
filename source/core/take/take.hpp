#pragma once

#include "rendering/context.hpp"
#include <string>

namespace take {

struct Take {
	std::string        scene;
	rendering::Context context;
};

}
