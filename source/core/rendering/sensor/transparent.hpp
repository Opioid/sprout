#ifndef SU_CORE_RENDERING_SENSOR_TRANSPARENT_HPP
#define SU_CORE_RENDERING_SENSOR_TRANSPARENT_HPP

#include "base/math/vector4.hpp"
#include "sensor.hpp"

namespace rendering::sensor {

class Transparent : public Sensor {
  public:
    Transparent(float exposure) noexcept;

    ~Transparent() noexcept override;

    void set_layer(int32_t layer) noexcept override final;

    void clear(float weight) noexcept override final;

    void set_weights(float weight) noexcept override final;

    bool has_alpha_transparency() const noexcept override final;

  protected:
    void add_pixel(int2 pixel, float4 const& color, float weight) noexcept override final;

    void add_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept override final;

    void splat_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept override final;

    void resolve(int32_t begin, int32_t end, image::Float4& target) const noexcept override final;

    void resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const
        noexcept override final;

    void on_resize(int2 dimensions, int32_t num_layers) noexcept override;

    struct Pixel {
        float4 color;
        float  weight;
    };

    Pixel* layers_;
    Pixel* pixels_;
};

}  // namespace rendering::sensor

#endif
