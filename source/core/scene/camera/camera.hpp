#ifndef SU_CORE_SCENE_CAMERA_CAMERA_HPP
#define SU_CORE_SCENE_CAMERA_CAMERA_HPP

#include "base/json/json_types.hpp"
#include "base/math/vector2.hpp"
#include "scene/prop/interface_stack.hpp"
#include "scene/scene_constants.hpp"

#include <string_view>

namespace math {
class Frustum;
}

namespace sampler {
class Sampler;
struct Camera_sample;
struct Camera_sample_to;
}  // namespace sampler

namespace rendering::sensor {
class Sensor;
}

namespace scene {

namespace entity {
struct Composed_transformation;
}  // namespace entity

struct Ray;
class Scene;
class Worker;

namespace camera {

class Camera {
  public:
    using Frustum        = math::Frustum;
    using Prop           = prop::Prop;
    using Transformation = entity::Composed_transformation;
    using Sample         = sampler::Camera_sample;
    using Sample_to      = sampler::Camera_sample_to;
    using Sampler        = sampler::Sampler;
    using Sensor         = rendering::sensor::Sensor;

    Camera(int2 resolution) noexcept;

    virtual ~Camera() noexcept;

    void init(uint32_t entity) noexcept;

    uint32_t entity() const noexcept;

    virtual uint32_t num_views() const noexcept = 0;

    virtual int2 sensor_dimensions() const noexcept = 0;

    virtual int4 view_bounds(uint32_t view) const noexcept = 0;

    virtual float pixel_solid_angle() const noexcept = 0;

    void update(Scene& scene, uint64_t time, Worker& worker) noexcept;

    virtual bool generate_ray(Sample const& sample, uint32_t frame, uint32_t view,
                              Scene const& scene, Ray& ray) const noexcept = 0;

    virtual bool sample(int4 const& bounds, uint64_t time, float3 const& p, Sampler& sampler,
                        uint32_t sampler_dimension, Scene const& scene, Sample_to& sample) const
        noexcept = 0;

    virtual Frustum frustum() const noexcept;

    void set_parameters(json::Value const& parameters) noexcept;

    int2 resolution() const noexcept;

    void set_resolution(int2 resolution) noexcept;

    Sensor& sensor() const noexcept;

    void set_sensor(Sensor* sensor) noexcept;

    prop::Interface_stack const& interface_stack() const noexcept;

    uint64_t frame_step() const noexcept;

    uint64_t frame_duration() const noexcept;

    uint64_t absolute_time(uint32_t frame, float frame_delta) const noexcept;

  protected:
    virtual void on_update(uint64_t time, Worker& worker) noexcept = 0;

    virtual void set_parameter(std::string_view name, json::Value const& value) noexcept = 0;

    static Ray create_ray(float3 const& origin, float3 const& direction, uint64_t time) noexcept;

    uint32_t entity_ = 0xFFFFFFFF;

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
