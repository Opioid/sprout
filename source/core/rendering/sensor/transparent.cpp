#include "transparent.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector4.inl"
#include "base/memory/align.hpp"
#include "image/typed_image.hpp"

namespace rendering::sensor {

Transparent::Transparent() : layers_(nullptr), pixels_(nullptr) {}

Transparent::~Transparent() {
    memory::free_aligned(layers_);
}

void Transparent::set_layer(int32_t layer) {
    pixels_ = layers_ + layer * (dimensions_[0] * dimensions_[1]);
}

void Transparent::clear(float weight) {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        pixels_[i].color  = float4(0.f);
        pixels_[i].weight = weight;
    }
}

void Transparent::set_weights(float weight) {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        pixels_[i].weight = weight;
    }
}

void Transparent::fix_zero_weights() {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        if (pixels_[i].weight <= 0.f) {
            pixels_[i].weight = 1.f;
        }
    }
}

bool Transparent::has_alpha_transparency() const {
    return true;
}

void Transparent::add_pixel(int2 pixel, float4 const& color, float weight) {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    value.color += weight * color;
    value.weight += weight;
}

void Transparent::add_pixel_atomic(int2 pixel, float4 const& color, float weight) {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    atomic::add_assign(value.color[0], weight * color[0]);
    atomic::add_assign(value.color[1], weight * color[1]);
    atomic::add_assign(value.color[2], weight * color[2]);
    atomic::add_assign(value.color[3], weight * color[3]);
    atomic::add_assign(value.weight, weight);
}

void Transparent::splat_pixel_atomic(int2 pixel, float4 const& color, float weight) {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    atomic::add_assign(value.color[0], weight * color[0]);
    atomic::add_assign(value.color[1], weight * color[1]);
    atomic::add_assign(value.color[2], weight * color[2]);
    atomic::add_assign(value.color[3], weight * color[3]);
}

void Transparent::resolve(int32_t begin, int32_t end, image::Float4& target) const {
    for (int32_t i = begin; i < end; ++i) {
        auto const& value = pixels_[i];

        float4 const color = value.color / value.weight;

        target.store(i, float4(color.xyz(), std::min(color[3], 1.f)));
    }
}

void Transparent::resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const {
    for (int32_t i = begin; i < end; ++i) {
        auto const& value = pixels_[i];

        float4 const color = value.color / value.weight;

        target.store(i, float4(color.xyz(), std::min(color[3], 1.f)));
    }
}

void Transparent::on_resize(int2 dimensions, int32_t num_layers) {
    int32_t const current_len = dimensions_[0] * dimensions_[1] * num_layers_;

    int32_t const len = dimensions[0] * dimensions[1] * num_layers;

    if (len != current_len) {
        memory::free_aligned(layers_);

        layers_ = memory::allocate_aligned<Pixel>(uint32_t(len));

        pixels_ = layers_;
    }
}

}  // namespace rendering::sensor
