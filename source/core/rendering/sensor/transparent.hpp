#ifndef SU_CORE_RENDERING_SENSOR_TRANSPARENT_HPP
#define SU_CORE_RENDERING_SENSOR_TRANSPARENT_HPP

#include "base/math/vector4.hpp"
#include "sensor.hpp"

namespace rendering::sensor {

class Transparent : public Sensor {
  public:
    Transparent(int2 dimensions, float exposure) noexcept;

    ~Transparent() noexcept override;

    void clear(float weigth) noexcept override final;

    bool has_alpha_transparency() const noexcept override final;

    size_t num_bytes() const noexcept override final;

  protected:
    void add_pixel(int2 pixel, float4 const& color, float weight) noexcept override final;

    void add_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept override final;

    void splat_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept override final;

    void resolve(int32_t begin, int32_t end, image::Float4& target) const noexcept override final;

    void resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const
        noexcept override final;

    struct Pixel {
        float4 color;
        float  weight;
    };

    Pixel* pixels_;
};

}  // namespace rendering::sensor

#endif
