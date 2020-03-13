#ifndef SU_CORE_SCENE_CAMERA_HEMISPHERICAL_HPP
#define SU_CORE_SCENE_CAMERA_HEMISPHERICAL_HPP

#include "camera.hpp"

namespace scene::camera {

class Hemispherical final : public Camera {
  public:
    uint32_t num_views() const final;

    int2 sensor_dimensions() const final;

    int2 view_offset(uint32_t view) const final;

    float pixel_solid_angle() const final;

    bool generate_ray(Sample const& sample, uint32_t frame, uint32_t view, Scene const& scene,
                      Ray& ray) const final;

    bool sample(uint32_t view, int4 const& bounds, uint64_t time, float3 const& p, Sampler& sampler,
                uint32_t sampler_dimension, Scene const& scene, Sample_to& sample) const final;

  private:
    void on_update(uint64_t time, Worker& worker) final;

    void set_parameter(std::string_view name, json::Value const& value) final;

    float d_x_;
    float d_y_;
};

}  // namespace scene::camera

#endif
