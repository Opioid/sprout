#pragma once

#include "base/math/vector3.hpp"

namespace scene {
class Scene;
namespace camera {
class Camera;
}
}  // namespace scene

namespace controller {

class Camera {
  public:
    Camera(uint32_t camera, scene::Scene const& scene);

    void mouse_delta(float3 delta, scene::Scene& scene);

  private:
    uint32_t camera_;

    float3 position_;
};

}  // namespace controller
