#include "camera_controller.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "core/scene/camera/camera.hpp"
#include "core/scene/prop/prop.hpp"
#include "core/scene/scene.hpp"

namespace controller {

Camera::Camera(uint32_t camera, scene::Scene const& scene) : camera_(camera) {
    scene::Scene::Transformation temp;
    auto const&                  transformation = scene.prop_transformation_at(camera_, 0, temp);

    position_ = transformation.position;
}

void Camera::mouse_delta(float3 delta, scene::Scene& scene) {
    position_.v[0] += 0.01f * delta[0];
    position_.v[1] += 0.01f * delta[1];
    position_.v[2] -= 0.0005f * delta[2];

    math::Transformation transformation{position_, float3(1.f), math::quaternion::identity()};

    scene.prop_set_transformation(camera_, transformation);
}

}  // namespace controller
