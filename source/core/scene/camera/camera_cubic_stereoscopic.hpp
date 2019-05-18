#ifndef SU_CORE_SCENE_CAMERA_CUBIC_STEREOSCOPIC_HPP
#define SU_CORE_SCENE_CAMERA_CUBIC_STEREOSCOPIC_HPP

#include "base/math/matrix3x3.hpp"
#include "camera_stereoscopic.hpp"

namespace scene::camera {

class Cubic_stereoscopic : public Stereoscopic {
  public:
    enum class Layout {
        lxlmxlylmylzlmzrxrmxryrmyrzrmz,
        rxlmxryrmyrzrmzlxlmxlylmylzlmz,
    };

    Cubic_stereoscopic(Layout layout, int2 resolution) noexcept;

    uint32_t num_views() const noexcept override final;

    int2 sensor_dimensions() const noexcept override final;

    int4 view_bounds(uint32_t view) const noexcept override final;

    float pixel_solid_angle() const noexcept override final;

    bool generate_ray(Prop const* self, Camera_sample const& sample, uint32_t frame, uint32_t view,
                      Ray& ray) const noexcept override final;

    void set_interpupillary_distance_falloff(float ipd_falloff) noexcept;

  private:
    void on_update(Prop const* self, uint64_t time, Worker& worker) noexcept override final;

    void set_parameter(std::string_view name, json::Value const& value) noexcept override final;

    float3 left_top_;
    float3 d_x_;
    float3 d_y_;

    int2 sensor_dimensions_;

    float ipd_falloff_;

    int4 view_bounds_[12];

    float3x3 view_rotations_[6];
};

}  // namespace scene::camera

#endif
