#include "camera.hpp"
#include "base/json/json.hpp"
#include "base/math/frustum.hpp"
#include "base/math/matrix3x3.inl"
#include "base/math/matrix4x4.inl"
#include "base/math/quaternion.inl"
#include "base/math/vector3.inl"
#include "rendering/rendering_worker.hpp"
#include "rendering/sensor/sensor.hpp"
#include "scene/prop/interface_stack.inl"
#include "scene/prop/prop_intersection.inl"
#include "scene/scene.hpp"
#include "scene/scene_constants.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"

namespace scene::camera {

Camera::Camera(int2 resolution) : resolution_(resolution), sensor_(nullptr) {}

Camera::~Camera() {
    delete sensor_;
}

void Camera::init(uint32_t entity) {
    entity_ = entity;
}

uint32_t Camera::entity() const {
    return entity_;
}

void Camera::update(Scene& scene, uint64_t time, Worker& worker) {
    interface_stack_.clear();
    interfaces_.clear();

    if (scene.has_volumes()) {
        Transformation temp;
        auto const&    transformation = scene.prop_transformation_at(entity_, time, temp);

        Ray ray(transformation.position, normalize(float3(1.f, 1.f, 1.f)), 0.f, Ray_max_t);

        prop::Intersection intersection;

        for (;;) {
            if (!scene.intersect_volume(ray, worker, intersection)) {
                break;
            }

            if (intersection.same_hemisphere(ray.direction)) {
                if (!interfaces_.remove(intersection)) {
                    interface_stack_.push(intersection);
                }
            } else {
                interfaces_.push(intersection);
            }

            ray.min_t = offset_f(ray.max_t);
            ray.max_t = Ray_max_t;
        }
    }

    on_update(time, worker);
}

Frustum Camera::frustum() const {
    return Frustum();
}

void Camera::set_parameters(json::Value const& parameters) {
    bool motion_blur = true;

    for (auto& n : parameters.GetObject()) {
        if ("frame_step" == n.name) {
            frame_step_ = uint64_t(float(scene::Units_per_second) * json::read_float(n.value));
        } else if ("frames_per_second" == n.name) {
            float const fps = json::read_float(n.value);
            if (0.f == fps) {
                frame_step_ = 0;
            } else {
                frame_step_ = scene::Units_per_second / uint64_t(fps);
            }
        } else if ("motion_blur" == n.name) {
            motion_blur = json::read_bool(n.value);
        } else {
            set_parameter(n.name.GetString(), n.value);
        }
    }

    frame_duration_ = motion_blur ? frame_step_ : 0;
}

int2 Camera::resolution() const {
    return resolution_;
}

rendering::sensor::Sensor& Camera::sensor() const {
    return *sensor_;
}

void Camera::set_sensor(Sensor* sensor) {
    sensor_ = sensor;
}

void Camera::set_resolution(int2 resolution) {
    resolution_ = resolution;
}

prop::Interface_stack const& Camera::interface_stack() const {
    return interface_stack_;
}

uint64_t Camera::frame_step() const {
    return frame_step_;
}

uint64_t Camera::frame_duration() const {
    return frame_duration_;
}

uint64_t Camera::absolute_time(uint32_t frame, float frame_delta) const {
    double const delta    = double(frame_delta);
    double const duration = double(frame_duration_);

    uint64_t const fdi = uint64_t(std::llrint(delta * duration));

    return uint64_t(frame) * frame_step_ + fdi;
}

Ray Camera::create_ray(float3 const& origin, float3 const& direction, uint64_t time) {
    return Ray(origin, direction, 0.f, Ray_max_t, 0, 0.f, time);
}

}  // namespace scene::camera
