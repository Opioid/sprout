#ifndef SU_CORE_SCENE_CAMERA_CAMERA_HPP
#define SU_CORE_SCENE_CAMERA_CAMERA_HPP

#include <memory>
#include <string>
#include <vector>
#include "base/math/vector2.hpp"
#include "scene/entity/entity.hpp"
#include "scene/prop/interface_stack.hpp"
#include "scene/scene_constants.hpp"

namespace sampler {
struct Camera_sample;
}

namespace rendering::sensor {
class Sensor;
}

namespace scene {

struct Ray;
class Scene;
class Worker;

namespace camera {

class Camera : public entity::Entity {
  public:
    using Camera_sample = sampler::Camera_sample;
    using Sensor        = rendering::sensor::Sensor;

    Camera(int2 resolution) noexcept;

    ~Camera() noexcept override;

    virtual uint32_t num_views() const noexcept = 0;

    virtual int2 sensor_dimensions() const noexcept = 0;

    virtual int4 view_bounds(uint32_t view) const noexcept = 0;

    virtual float pixel_solid_angle() const noexcept = 0;

    void update(Scene const& scene, Worker& worker) noexcept;

    virtual bool generate_ray(Camera_sample const& sample, uint32_t frame, uint32_t view,
                              Ray& ray) const noexcept = 0;

    void set_parameters(json::Value const& parameters) noexcept override final;

    int2 resolution() const noexcept;

    Sensor& sensor() const noexcept;

    void set_sensor(std::unique_ptr<Sensor> sensor) noexcept;

    prop::Interface_stack const& interface_stack() const noexcept;

    float    frame_duration() const noexcept;
    uint64_t frame_duration_i() const noexcept;

    void set_frame_duration(float frame_duration) noexcept;

    uint64_t absolute_time(uint32_t frame, float frame_delta) const noexcept;

    bool motion_blur() const noexcept;

    void set_motion_blur(bool motion_blur) noexcept;

  protected:
    virtual void on_update(Worker& worker) noexcept = 0;

    virtual void set_parameter(std::string_view name, json::Value const& value) noexcept = 0;

    void on_set_transformation() noexcept override final;

    static Ray create_ray(float3 const& origin, float3 const& direction, uint64_t time) noexcept;

    int2 resolution_;

    std::unique_ptr<Sensor> sensor_;

    prop::Interface_stack interface_stack_;
    prop::Interface_stack interfaces_;

    int32_t filter_radius_ = 0;

    float frame_duration_ = 1.f / 60.f;

    uint64_t frame_duration_i_ = scene::Units_per_second / 60;

    bool motion_blur_ = true;
};

}  // namespace camera
}  // namespace scene

#endif
