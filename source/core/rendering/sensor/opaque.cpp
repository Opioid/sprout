#include "opaque.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "image/typed_image.inl"

namespace rendering::sensor {

Opaque::Opaque(int2 dimensions, float exposure) noexcept
    : Sensor(dimensions, exposure),
      pixels_(memory::allocate_aligned<float4>(dimensions[0] * dimensions[1])) {}

Opaque::~Opaque() noexcept {
    memory::free_aligned(pixels_);
}

void Opaque::clear() noexcept {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        pixels_[i] = float4(0.f);
    }
}

bool Opaque::has_alpha_transparency() const noexcept {
    return false;
}

size_t Opaque::num_bytes() const noexcept {
    auto const d = dimensions();
    return static_cast<size_t>(d[0] * d[1]) * sizeof(float4);
}

void Opaque::add_pixel(int2 pixel, float4 const& color, float weight) noexcept {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    value += float4(weight * color.xyz(), weight);
}

void Opaque::add_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    atomic::add_assign(value[0], weight * color[0]);
    atomic::add_assign(value[1], weight * color[1]);
    atomic::add_assign(value[2], weight * color[2]);
    atomic::add_assign(value[3], weight);
}

void Opaque::resolve(int32_t begin, int32_t end, image::Float4& target) const noexcept {
    float const exposure_factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        auto const& value = pixels_[i];

        float3 const color = value.xyz() / value[3];

        target.at(i) = float4(exposure_factor * color, 1.f);
    }
}

}  // namespace rendering::sensor
