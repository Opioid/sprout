#include "camera_controller.hpp"
#include "core/scene/camera/camera.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"

namespace controller {

Camera::Camera(std::shared_ptr<scene::camera::Camera> camera) : camera_(camera) {
	scene::entity::Composed_transformation temp;
	auto& transformation = camera_->transformation_at(0.f, temp);

	position_ = transformation.position;
}

void Camera::mouse_delta(float3 delta) {
	position_.v[0] += 0.01f * delta[0];
	position_.v[1] += 0.01f * delta[1];
	position_.v[2] -= 0.0005f * delta[2];

	math::Transformation transformation {
		position_,
		float3(1.f),
		math::quaternion::identity()
	};

	camera_->set_transformation(transformation);
}

}
