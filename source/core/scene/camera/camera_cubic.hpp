#ifndef SU_CORE_SCENE_CAMERA_CUBIC_HPP
#define SU_CORE_SCENE_CAMERA_CUBIC_HPP

#include "camera.hpp"

namespace scene::camera {

class Cubic : public Camera {
  public:
    enum class Layout { xmxymyzmz, xmxy_myzmz };

    Cubic(Layout layout, int2 resolution);

    virtual uint32_t num_views() const override final;

    virtual int2 sensor_dimensions() const override final;

    virtual int4 view_bounds(uint32_t view) const override final;

    virtual float pixel_solid_angle() const override final;

    virtual bool generate_ray(sampler::Camera_sample const& sample, uint32_t view,
                              scene::Ray& ray) const override final;

  private:
    virtual void on_update(Worker& worker) override final;

    virtual void set_parameter(std::string_view name, json::Value const& value) override final;

    float3 left_top_;
    float3 d_x_;
    float3 d_y_;

    int2 sensor_dimensions_;

    int4 view_bounds_[6];

    float3x3 view_rotations_[6];
};

}  // namespace scene::camera

#endif
