#pragma once

#include "base/math/vector3.hpp"
#include <memory>

namespace scene { namespace camera { class Camera; } }

namespace controller {

class Camera {

public:

	Camera(std::shared_ptr<scene::camera::Camera> camera);

	void mouse_delta(int2 delta);

private:

	std::shared_ptr<scene::camera::Camera> camera_;

	float3 position_;
};

}
