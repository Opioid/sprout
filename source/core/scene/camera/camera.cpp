#include "camera.hpp"
#include "rendering/film/film.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace camera {

Camera::Camera(const math::float2& dimensions, rendering::film::Film* film, float frame_duration) :
	dimensions_(calculate_dimensions(dimensions, film)), film_(film), frame_duration_(frame_duration) {}

Camera::~Camera() {
	delete film_;
}

rendering::film::Film& Camera::film() const {
	return *film_;
}

float Camera::frame_duration() const {
	return frame_duration_;
}

void Camera::on_set_transformation() {}

math::float2 Camera::calculate_dimensions(const math::float2& dimensions, rendering::film::Film* film) {
	if (0.f == dimensions.x && 0.f == dimensions.y) {
		return math::float2(static_cast<float>(film->dimensions().x), static_cast<float>(film->dimensions().y));
	} else if (0.f == dimensions.x) {
		float x_y_ratio = static_cast<float>(film->dimensions().x) / static_cast<float>(film->dimensions().y);
		return math::float2(dimensions.y * x_y_ratio, dimensions.y);
	} else if (0.f == dimensions.y) {
		float y_x_ratio = static_cast<float>(film->dimensions().y) / static_cast<float>(film->dimensions().x);
		return math::float2(dimensions.x, dimensions.x * y_x_ratio);
	}

	return dimensions;
}

}}
