#include "camera.hpp"
#include "rendering/sensor/sensor.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace camera {

Camera::Camera(math::float2 dimensions, rendering::sensor::Sensor* sensor, float ray_max_t,
			   float frame_duration, bool motion_blur) :
	dimensions_(calculate_dimensions(dimensions, sensor)), film_(sensor), ray_max_t_(ray_max_t),
	frame_duration_(frame_duration), motion_blur_(motion_blur) {}

Camera::~Camera() {
	delete film_;
}

rendering::sensor::Sensor& Camera::sensor() const {
	return *film_;
}

float Camera::frame_duration() const {
	return frame_duration_;
}

bool Camera::motion_blur() const {
	return motion_blur_;
}

void Camera::on_set_transformation() {}

math::float2 Camera::calculate_dimensions(math::float2 dimensions, rendering::sensor::Sensor* sensor) {
	if (0.f == dimensions.x && 0.f == dimensions.y) {
		return math::float2(sensor->dimensions());
	} else if (0.f == dimensions.x) {
		math::float2 fd(sensor->dimensions());
		float x_y_ratio = fd.x / fd.y;
		return math::float2(dimensions.y * x_y_ratio, dimensions.y);
	} else if (0.f == dimensions.y) {
		math::float2 fd(sensor->dimensions());
		float y_x_ratio = fd.y / fd.x;
		return math::float2(dimensions.x, dimensions.x * y_x_ratio);
	}

	return dimensions;
}

}}
