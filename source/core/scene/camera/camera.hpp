#ifndef SU_CORE_SCENE_CAMERA_CAMERA_HPP
#define SU_CORE_SCENE_CAMERA_CAMERA_HPP

#include <string_view>
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

    void update(Scene& scene, uint64_t time, Worker& worker) noexcept;

    virtual bool generate_ray(Camera_sample const& sample, uint32_t frame, uint32_t view,
                              Ray& ray) const noexcept = 0;

    void set_parameters(json::Value const& parameters) noexcept override final;

    int2 resolution() const noexcept;

    Sensor& sensor() const noexcept;

    void set_sensor(Sensor* sensor) noexcept;

    prop::Interface_stack const& interface_stack() const noexcept;

    uint64_t frame_step() const noexcept;
    uint64_t frame_duration() const noexcept;

    uint64_t absolute_time(uint32_t frame, float frame_delta) const noexcept;

  protected:
    virtual void on_update(uint64_t time, Worker& worker) noexcept = 0;

    virtual void set_parameter(std::string_view name, json::Value const& value) noexcept = 0;

    void on_set_transformation() noexcept override final;

    static Ray create_ray(float3 const& origin, float3 const& direction, uint64_t time) noexcept;

    int2 resolution_;

    Sensor* sensor_;

    prop::Interface_stack interface_stack_;
    prop::Interface_stack interfaces_;

    int32_t filter_radius_ = 0;

    uint64_t frame_step_     = scene::Units_per_second / 60;
    uint64_t frame_duration_ = frame_step_;
};

}  // namespace camera
}  // namespace scene

#endif
