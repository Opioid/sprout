#include "transparent.hpp"
#include "base/atomic/atomic.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace rendering::sensor {

Transparent::Transparent(int32_t filter_radius, bool adaptive)
    : Sensor(filter_radius, adaptive),
      layer_weights_(nullptr),
      pixel_weights_(nullptr),
      layers_(nullptr),
      pixels_(nullptr) {}

Transparent::~Transparent() {
    delete[] layer_weights_;
    delete[] layers_;
}

void Transparent::set_layer(int32_t layer) {
    pixel_weights_ = layer_weights_ + layer * (dimensions_[0] * dimensions_[1]);

    pixels_ = layers_ + layer * (dimensions_[0] * dimensions_[1]);
}

void Transparent::set_weights(float weight) {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        pixel_weights_[i] = weight;
    }
}

void Transparent::fix_zero_weights() {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        if (pixel_weights_[i] <= 0.f) {
            pixel_weights_[i] = 1.f;
        }
    }
}

bool Transparent::has_alpha_transparency() const {
    return true;
}

void Transparent::add_pixel(int2 pixel, float4 const& color, float weight) {
    auto const d = dimensions();

    int32_t const i = d[0] * pixel[1] + pixel[0];

    pixel_weights_[i] += weight;

    pixels_[i] += weight * color;
}

void Transparent::add_pixel_atomic(int2 pixel, float4 const& color, float weight) {
    auto const d = dimensions();

    int32_t const i = d[0] * pixel[1] + pixel[0];

    atomic::add_assign(pixel_weights_[i], weight);

    float4& value = pixels_[i];
    atomic::add_assign(value[0], weight * color[0]);
    atomic::add_assign(value[1], weight * color[1]);
    atomic::add_assign(value[2], weight * color[2]);
    atomic::add_assign(value[3], weight * color[3]);
}

void Transparent::splat_pixel_atomic(int2 pixel, float4 const& color, float weight) {
    auto const d = dimensions();

    float4& value = pixels_[d[0] * pixel[1] + pixel[0]];
    atomic::add_assign(value[0], weight * color[0]);
    atomic::add_assign(value[1], weight * color[1]);
    atomic::add_assign(value[2], weight * color[2]);
    atomic::add_assign(value[3], weight * color[3]);
}

void Transparent::resolve(int32_t begin, int32_t end, image::Float4& target) const {
    for (int32_t i = begin; i < end; ++i) {
        float4 const color = pixels_[i] / pixel_weights_[i];

        target.store(i, float4(color.xyz(), std::min(color[3], 1.f)));
    }
}

void Transparent::resolve(int32_t begin, int32_t end, uint32_t slot, AOV property,
                          image::Float4& target) const {
    if (aov::accumulating(property)) {
        for (int32_t i = begin; i < end; ++i) {
            float const weight = pixel_weights_[i];

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

void Transparent::resolve_accumulate(int32_t begin, int32_t end, image::Float4& target) const {
    for (int32_t i = begin; i < end; ++i) {
        float4 const color = pixels_[i] / pixel_weights_[i];

        target.store(i, float4(color.xyz(), std::min(color[3], 1.f)));
    }
}

void Transparent::on_resize(int2 dimensions, int32_t num_layers) {
    uint32_t const current_len = uint32_t(dimensions_[0] * dimensions_[1] * num_layers_);

    uint32_t const len = uint32_t(dimensions[0] * dimensions[1] * num_layers);

    if (len != current_len) {
        delete[] layer_weights_;
        delete[] layers_;

        layer_weights_ = new float[len];
        pixel_weights_ = layer_weights_;

        layers_ = new float4[len];
        pixels_ = layers_;
    }
}

void Transparent::on_clear(float weight) {
    auto const d = dimensions();
    for (int32_t i = 0, len = d[0] * d[1]; i < len; ++i) {
        pixel_weights_[i] = weight;
        pixels_[i]        = float4(0.f);
    }
}

}  // namespace rendering::sensor
