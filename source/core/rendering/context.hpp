#pragma once

#include "scene/camera/camera.hpp"
#include <memory>

namespace rendering {

struct Context {
	std::shared_ptr<camera::Camera>	camera;
};

}
