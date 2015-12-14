#include "camera.hpp"
#include "rendering/sensor/sensor.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace camera {

Camera::Camera(math::int2 resolution, float ray_max_t, float frame_duration, bool motion_blur) :
	resolution_(resolution),
	sensor_(nullptr),
	seeds_(new math::uint2[resolution.x * resolution.y]),
	ray_max_t_(ray_max_t),
	frame_duration_(frame_duration),
	motion_blur_(motion_blur) {}

Camera::~Camera() {
	delete [] seeds_;

	delete sensor_;
}

math::int2 Camera::resolution() const {
	return resolution_;
}

rendering::sensor::Sensor& Camera::sensor() const {
	return *sensor_;
}

void Camera::set_sensor(rendering::sensor::Sensor* sensor) {
	sensor_ = sensor;
}

math::uint2 Camera::seed(int32_t x, int32_t y) const {
	return seeds_[resolution_.x * y + x];
}

void Camera::set_seed(int32_t x, int32_t y, math::uint2 seed) {
	seeds_[resolution_.x * y + x] = seed;
}

float Camera::frame_duration() const {
	return frame_duration_;
}

bool Camera::motion_blur() const {
	return motion_blur_;
}

void Camera::on_set_transformation() {}

}}
