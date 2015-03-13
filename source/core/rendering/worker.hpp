#pragma once

#include "rectangle.hpp"

namespace scene {

class Scene;

}

namespace camera {

class Camera;

}

namespace rendering {

class Worker {
public:

	void render(const scene::Scene& scene, const camera::Camera& camera, const Rectui& tile) const;
};

}
