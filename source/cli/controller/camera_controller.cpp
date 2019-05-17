#include "camera_controller.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "core/scene/camera/camera.hpp"
#include "core/scene/entity/entity.hpp"
#include "core/scene/scene.hpp"

namespace controller {

Camera::Camera(uint32_t camera, scene::Scene const& scene) : camera_(camera) {
    scene::entity::Composed_transformation temp;
    auto& transformation = scene.entity(camera_)->transformation_at(0, temp);

    position_ = transformation.position;
}

void Camera::mouse_delta(float3 delta, scene::Scene& scene) {
    position_.v[0] += 0.01f * delta[0];
    position_.v[1] += 0.01f * delta[1];
    position_.v[2] -= 0.0005f * delta[2];

    math::Transformation transformation{position_, float3(1.f), math::quaternion::identity()};

    scene.entity(camera_)->set_transformation(transformation);
}

}  // namespace controller
