#ifndef SU_CORE_SCENE_CAMERA_PERSPECTIVE_STEREOSCOPIC_HPP
#define SU_CORE_SCENE_CAMERA_PERSPECTIVE_STEREOSCOPIC_HPP

#include "camera_stereoscopic.hpp"

namespace scene::camera {

class Perspective_stereoscopic : public Stereoscopic {
  public:
    Perspective_stereoscopic(int2 resolution) noexcept;

    uint32_t num_views() const noexcept override final;

    int2 sensor_dimensions() const noexcept override final;

    int4 view_bounds(uint32_t view) const noexcept override final;

    float pixel_solid_angle() const noexcept override final;

    bool generate_ray(Camera_sample const& sample, uint32_t frame, uint32_t view, Ray& ray) const
        noexcept override final;

    void set_fov(float fov) noexcept;

  private:
    void on_update(uint64_t time, Worker& worker) noexcept override final;

    void set_parameter(std::string_view name, json::Value const& value) noexcept override final;

    float3 left_top_;
    float3 d_x_;
    float3 d_y_;

    int4 view_bounds_[2];
};

}  // namespace scene::camera

#endif
