#ifndef SU_CORE_SCENE_CAMERA_SPHERCICAL_STEREOSCOPIC_HPP
#define SU_CORE_SCENE_CAMERA_SPHERCICAL_STEREOSCOPIC_HPP

#include "base/math/vector4.hpp"
#include "camera_stereoscopic.hpp"

namespace scene::camera {

class Spherical_stereoscopic : public Stereoscopic {
  public:
    Spherical_stereoscopic(int2 resolution) noexcept;

    uint32_t num_views() const noexcept override final;

    int2 sensor_dimensions() const noexcept override final;

    int4 view_bounds(uint32_t view) const noexcept override final;

    float pixel_solid_angle() const noexcept override final;

    bool generate_ray(Prop const* self, Camera_sample const& sample, uint32_t frame, uint32_t view,
                      Ray& ray) const noexcept override final;

  private:
    void on_update(Prop const* self, uint64_t time, Worker& worker) noexcept override final;

    void set_parameter(std::string_view name, json::Value const& value) noexcept override final;

    float d_x_;
    float d_y_;

    int4 view_bounds_[2];
};

}  // namespace scene::camera

#endif
