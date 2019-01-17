#pragma once

#include "base/math/vector3.hpp"

namespace scene {
namespace camera {
class Camera;
}
}  // namespace scene

namespace controller {

class Camera {
  public:
    Camera(scene::camera::Camera& camera);

    void mouse_delta(float3 delta);

  private:
    scene::camera::Camera& camera_;

    float3 position_;
};

}  // namespace controller
