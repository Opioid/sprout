#ifndef SU_CORE_RENDERING_SENSOR_OPAQUE_HPP
#define SU_CORE_RENDERING_SENSOR_OPAQUE_HPP

#include "sensor.hpp"

namespace rendering::sensor {

class Opaque : public Sensor {
  public:
    Opaque(int2 dimensions, float exposure) noexcept;

    ~Opaque() noexcept override;

    void clear() noexcept override final;

    bool has_alpha_transparency() const noexcept override final;

    size_t num_bytes() const noexcept override final;

  protected:
    void add_pixel(int2 pixel, float4 const& color, float weight) noexcept override final;

    void add_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept override final;

    void splat_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept override final;

    void resolve(int32_t begin, int32_t end, image::Float4& target) const noexcept override final;

    void resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const
        noexcept override final;

    // weight_sum is saved in pixel.w
    float4* pixels_;
};

}  // namespace rendering::sensor

#endif
