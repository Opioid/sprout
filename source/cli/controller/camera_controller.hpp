#pragma once

#include <memory>
#include "base/math/vector3.hpp"

namespace scene {
namespace camera {
class Camera;
}
}  // namespace scene

namespace controller {

class Camera {
  public:
    Camera(std::shared_ptr<scene::camera::Camera> camera);

    void mouse_delta(float3 delta);

  private:
    std::shared_ptr<scene::camera::Camera> camera_;

    float3 position_;
};

}  // namespace controller
