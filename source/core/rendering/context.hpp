#pragma once

#include "scene/camera/camera.hpp"
#include <memory>

namespace rendering {

struct Context {
	std::shared_ptr<scene::camera::Camera> camera;
};

}
