#ifndef SU_CORE_RENDERING_SENSOR_TRANSPARENT_HPP
#define SU_CORE_RENDERING_SENSOR_TRANSPARENT_HPP

#include "base/math/vector4.hpp"
#include "sensor.hpp"

namespace rendering::sensor {

class Transparent : public Sensor {
  public:
    Transparent(int32_t filter_radius);

    ~Transparent() override;

    void set_layer(int32_t layer) final;

    void set_weights(float weight) final;

    void fix_zero_weights() final;

  protected:
    void add_pixel(int2 pixel, float4_p color, float weight) final;

    void add_pixel_atomic(int2 pixel, float4_p color, float weight) final;

    void splat_pixel_atomic(int2 pixel, float4_p color, float weight) final;

    void resolve(int32_t begin, int32_t end, image::Float4& target) const final;

    void resolve(int32_t begin, int32_t end, uint32_t slot, AOV property,
                 image::Float4& target) const final;

    void resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const final;

    void on_resize(int2 dimensions, int32_t num_layers) final;

    void on_clear(float weight) final;

    float* layer_weights_;
    float* pixel_weights_;

    float4* layers_;
    float4* pixels_;
};

}  // namespace rendering::sensor

#endif
