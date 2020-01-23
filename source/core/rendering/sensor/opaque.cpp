#include "opaque.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "image/typed_image.hpp"

namespace rendering::sensor {

Opaque::Opaque(float exposure) noexcept : Sensor(exposure), layers_(nullptr), pixels_(nullptr) {}

Opaque::~Opaque() noexcept {
    memory::free_aligned(layers_);
}

void Opaque::set_layer(int32_t layer) noexcept {
    pixels_ = layers_ + layer * (dimensions_[0] * dimensions_[1]);
}

void Opaque::clear(float weight) noexcept {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1] * num_layers_; i < len; ++i) {
        pixels_[i] = float4(0.f, 0.f, 0.f, weight);
    }
}

void Opaque::set_weights(float weight) noexcept {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        pixels_[i][3] = weight;
    }
}

bool Opaque::has_alpha_transparency() const noexcept {
    return false;
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

void Opaque::splat_pixel_atomic(int2 pixel, float4 const& color, float weight) noexcept {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    atomic::add_assign(value[0], weight * color[0]);
    atomic::add_assign(value[1], weight * color[1]);
    atomic::add_assign(value[2], weight * color[2]);
}

void Opaque::resolve(int32_t begin, int32_t end, image::Float4& target) const noexcept {
    float const exposure_factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        auto const& value = pixels_[i];

        float3 const color = value.xyz() / value[3];

        target.store(i, float4(exposure_factor * color, 1.f));
    }
}

void Opaque::resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const noexcept {
    float const exposure_factor = exposure_factor_;

    for (int32_t i = begin; i < end; ++i) {
        auto const& value = pixels_[i];

        float3 const color = value.xyz() / value[3];

        float3 const old = target.load(i).xyz();

        target.store(i, float4(old + exposure_factor * color, 1.f));
    }
}

void Opaque::on_resize(int2 dimensions, int32_t num_layers) noexcept {
    int32_t const current_len = dimensions_[0] * dimensions_[1] * num_layers_;

    int32_t const len = dimensions[0] * dimensions[1] * num_layers;

    if (len != current_len) {
        memory::free_aligned(layers_);

        layers_ = memory::allocate_aligned<float4>(uint32_t(len));

        pixels_ = layers_;
    }
}

}  // namespace rendering::sensor
