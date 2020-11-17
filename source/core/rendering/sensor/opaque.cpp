#include "opaque.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace rendering::sensor {

Opaque::Opaque(int32_t filter_radius)
    : Sensor(filter_radius, false), layers_(nullptr), pixels_(nullptr) {}

Opaque::~Opaque() {
    delete[] layers_;
}

void Opaque::set_layer(int32_t layer) {
    pixels_ = layers_ + layer * (dimensions_[0] * dimensions_[1]);
}

void Opaque::set_weights(float weight) {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        pixels_[i][3] = weight;
    }
}

void Opaque::fix_zero_weights() {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        if (pixels_[i][3] <= 0.f) {
            pixels_[i][3] = 1.f;
        }
    }
}

void Opaque::add_pixel(int2 pixel, float4_p color, float weight) {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    value += float4(weight * color.xyz(), weight);
}

void Opaque::add_pixel_atomic(int2 pixel, float4_p color, float weight) {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    atomic::add_assign(value[0], weight * color[0]);
    atomic::add_assign(value[1], weight * color[1]);
    atomic::add_assign(value[2], weight * color[2]);
    atomic::add_assign(value[3], weight);
}

void Opaque::splat_pixel_atomic(int2 pixel, float4_p color, float weight) {
    auto const d = dimensions();

    auto& value = pixels_[d[0] * pixel[1] + pixel[0]];
    atomic::add_assign(value[0], weight * color[0]);
    atomic::add_assign(value[1], weight * color[1]);
    atomic::add_assign(value[2], weight * color[2]);
}

void Opaque::resolve(int32_t begin, int32_t end, image::Float4& target) const {
    for (int32_t i = begin; i < end; ++i) {
        auto const& value = pixels_[i];

        float3 const color = value.xyz() / value[3];

        target.store(i, float4(color, 1.f));
    }
}

void Opaque::resolve(int32_t begin, int32_t end, uint32_t slot, AOV property,
                     image::Float4& target) const {
    if (aov::accumulating(property)) {
        for (int32_t i = begin; i < end; ++i) {
            float const weight = pixels_[i][3];

            float3 const color = aov_.value(i, slot) / weight;

            target.store(i, float4(color, 0.f));
        }
    } else {
        for (int32_t i = begin; i < end; ++i) {
            float3 const color = aov_.value(i, slot);

            target.store(i, float4(color, 0.f));
        }
    }
}

void Opaque::resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const {
    for (int32_t i = begin; i < end; ++i) {
        auto const& value = pixels_[i];

        float3 const color = value.xyz() / value[3];

        float3 const old = target.at(i).xyz();

        target.store(i, float4(old + color, 1.f));
    }
}

void Opaque::on_resize(int2 dimensions, int32_t num_layers) {
    int32_t const current_len = dimensions_[0] * dimensions_[1] * num_layers_;

    int32_t const len = dimensions[0] * dimensions[1] * num_layers;

    if (len != current_len) {
        delete[] layers_;

        layers_ = new float4[uint32_t(len)];

        pixels_ = layers_;
    }
}

void Opaque::on_clear(float weight) {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1] * num_layers_; i < len; ++i) {
        pixels_[i] = float4(0.f, 0.f, 0.f, weight);
    }
}

}  // namespace rendering::sensor
