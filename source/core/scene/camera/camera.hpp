#ifndef SU_CORE_SCENE_CAMERA_CAMERA_HPP
#define SU_CORE_SCENE_CAMERA_CAMERA_HPP

#include "base/json/json_types.hpp"
#include "base/math/vector4.hpp"
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

    Camera();

    virtual ~Camera();

    void init(uint32_t entity);

    uint32_t entity() const;

    virtual uint32_t num_views() const = 0;

    virtual int2 sensor_dimensions() const = 0;

    virtual int4 view_bounds(uint32_t view) const = 0;

    virtual float pixel_solid_angle() const = 0;

    void update(Scene& scene, uint64_t time, Worker& worker);

    virtual bool generate_ray(Sample const& sample, uint32_t frame, uint32_t view,
                              Scene const& scene, Ray& ray) const = 0;

    virtual bool sample(int4 const& bounds, uint64_t time, float3 const& p, Sampler& sampler,
                        uint32_t sampler_dimension, Scene const& scene,
                        Sample_to& sample) const = 0;

    virtual Frustum frustum() const;

    void set_parameters(json::Value const& parameters);

    int2 resolution() const;

    int4 const& crop() const;

    void set_resolution(int2 resolution, int4 const& crop);

    Sensor& sensor() const;

    void set_sensor(Sensor* sensor);

    prop::Interface_stack const& interface_stack() const;

    uint64_t frame_step() const;

    uint64_t frame_duration() const;

    uint64_t absolute_time(uint32_t frame, float frame_delta) const;

  protected:
    virtual void on_update(uint64_t time, Worker& worker) = 0;

    virtual void set_parameter(std::string_view name, json::Value const& value) = 0;

    static Ray create_ray(float3 const& origin, float3 const& direction, uint64_t time);

    uint32_t entity_ = 0xFFFFFFFF;

    int32_t filter_radius_ = 0;

    int2 resolution_ = int2(0);

    int4 crop_ = int4(0);

    Sensor* sensor_ = nullptr;

    prop::Interface_stack interface_stack_;
    prop::Interface_stack interfaces_;

    uint64_t frame_step_     = scene::Units_per_second / 60;
    uint64_t frame_duration_ = frame_step_;
};

}  // namespace camera
}  // namespace scene

#endif
