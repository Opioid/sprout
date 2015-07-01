#include "camera.hpp"
#include "rendering/film/film.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace camera {

Camera::Camera(const math::float2& dimensions, rendering::film::Film* film, float shutter_duration) :
	dimensions_(calculate_dimensions(dimensions, film)), film_(film), shutter_duration_(shutter_duration) {}

Camera::~Camera() {
	delete film_;
}

rendering::film::Film& Camera::film() const {
	return *film_;
}

float Camera::shutter_duration() const {
	return shutter_duration_;
}

void Camera::on_set_transformation() {}

math::float2 Camera::calculate_dimensions(const math::float2& dimensions, rendering::film::Film* film) {
	if (0.f == dimensions.x && 0.f == dimensions.y) {
		return math::float2(static_cast<float>(film->dimensions().x), static_cast<float>(film->dimensions().y));
	} else if (0.f == dimensions.x) {
		return math::float2(dimensions.y * (static_cast<float>(film->dimensions().x) / static_cast<float>(film->dimensions().y)), dimensions.y);
	} else if (0.f == dimensions.y) {
		return math::float2(dimensions.x, dimensions.x * (static_cast<float>(film->dimensions().y) / static_cast<float>(film->dimensions().x)));
	}

	return dimensions;
}

}}
