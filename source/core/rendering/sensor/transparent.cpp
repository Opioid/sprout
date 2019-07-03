#include "transparent.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "image/typed_image.hpp"

namespace rendering::sensor {

Transparent::Transparent(int2 dimensions, float exposure) noexcept
    : Sensor(dimensions, exposure),
      pixels_(memory::allocate_aligned<Pixel>(dimensions[0] * dimensions[1])) {}

Transparent::~Transparent() noexcept {
    memory::free_aligned(pixels_);
}

void Transparent::clear(float weigth) noexcept {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        pixels_[i].color  = float4(0.f);
        pixels_[i].weight = weigth;
    }
}

bool Transparent::has_alpha_transparency() const noexcept {
    return true;
}

size_t Transparent::num_bytes() const noexcept {
    auto const d = dimensions();
    return d[0] * d[1] * sizeof(Pixel);
}

void Transparent::add_pixel(int2 pixel, float4 const& color, float weight) noexcept {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    value.color += weight * color;
    value.weight += weight;
}

void Transparent::add_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    atomic::add_assign(value.color[0], weight * color[0]);
    atomic::add_assign(value.color[1], weight * color[1]);
    atomic::add_assign(value.color[2], weight * color[2]);
    atomic::add_assign(value.color[3], weight * color[3]);
    atomic::add_assign(value.weight, weight);
}

void Transparent::splat_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    atomic::add_assign(value.color[0], weight * color[0]);
    atomic::add_assign(value.color[1], weight * color[1]);
    atomic::add_assign(value.color[2], weight * color[2]);
    atomic::add_assign(value.color[3], weight * color[3]);
}

void Transparent::resolve(int32_t begin, int32_t end, image::Float4& target) const noexcept {
    float const exposure_factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        auto const& value = pixels_[i];

        float4 const color = value.color / value.weight;

        target.store(i, float4(exposure_factor * color.xyz(), std::min(color[3], 1.f)));
    }
}

void Transparent::resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const
    noexcept {
    float const exposure_factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        auto const& value = pixels_[i];

        float4 const color = value.color / value.weight;

        target.store(i, float4(exposure_factor * color.xyz(), std::min(color[3], 1.f)));
    }
}

}  // namespace rendering::sensor
