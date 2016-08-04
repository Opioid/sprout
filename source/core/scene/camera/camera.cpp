#include "camera.hpp"
#include "rendering/sensor/sensor.hpp"
#include "base/json/json.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"
#include "base/math/quaternion.inl"

namespace scene { namespace camera {

Camera::Camera(int2 resolution, float ray_max_t) :
	resolution_(resolution),
	sensor_(nullptr),
	seed_dimensions_(0, 0),
	seeds_(nullptr),
	filter_radius_(0),
	ray_max_t_(ray_max_t),
	frame_duration_(0.f),
	motion_blur_(true) {}

Camera::~Camera() {
	delete [] seeds_;

	delete sensor_;
}

void Camera::set_parameters(const json::Value& parameters) {
	for (auto n = parameters.MemberBegin(); n != parameters.MemberEnd(); ++n) {
		const std::string node_name = n->name.GetString();
		const json::Value& node_value = n->value;

		if ("frame_duration" == node_name) {
			frame_duration_ = json::read_float(node_value);
		} else if ("frames_per_second" == node_name) {
			float fps = json::read_float(node_value);
			if (0.f == fps) {
				frame_duration_ = 0.f;
			} else {
				frame_duration_ = 1.f / fps;
			}
		} else if ("motion_blur" == node_name) {
			motion_blur_ = json::read_bool(node_value);
		} else {
			set_parameter(node_name, node_value);
		}
	}
}

int2 Camera::resolution() const {
	return resolution_;
}

rendering::sensor::Sensor& Camera::sensor() const {
	return *sensor_;
}

void Camera::set_sensor(rendering::sensor::Sensor* sensor) {
	delete sensor_;
	sensor_ = sensor;

	filter_radius_ = sensor->filter_radius_int();
	const int2 seed_dimensions = resolution_ + int2(2 * filter_radius_, 2 * filter_radius_);

	if (seed_dimensions != seed_dimensions_) {
		seed_dimensions_ = seed_dimensions;
		delete [] seeds_;
		seeds_ = new uint2[seed_dimensions.x * seed_dimensions.y];
	}
}

math::uint2 Camera::seed(int2 pixel) const {
	return seeds_[seed_dimensions_.x * (pixel.y + filter_radius_) + pixel.x + filter_radius_];
}

void Camera::set_seed(int2 pixel, uint2 seed) {
	seeds_[seed_dimensions_.x * (pixel.y + filter_radius_) + pixel.x + filter_radius_] = seed;
}

float Camera::frame_duration() const {
	return frame_duration_;
}

void Camera::set_frame_duration(float frame_duration) {
	frame_duration_ = frame_duration;
}

bool Camera::motion_blur() const {
	return motion_blur_;
}

void Camera::set_motion_blur(bool motion_blur) {
	motion_blur_ = motion_blur;
}

void Camera::on_set_transformation() {}

}}
