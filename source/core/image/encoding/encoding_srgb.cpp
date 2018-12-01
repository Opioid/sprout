#include "encoding_srgb.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/vector3.inl"
#include "base/random/generator.inl"
#include "base/spectrum/rgb.hpp"
#include "image/typed_image.inl"

namespace image::encoding {

Srgb::Srgb(int2 dimensions, bool dither)
    : rgb_(new byte3[static_cast<size_t>(dimensions[0] * dimensions[1])]), dither_(dither) {}

Srgb::~Srgb() {
    delete[] rgb_;
}

byte3 const* Srgb::data() const {
    return rgb_;
}

void Srgb::to_sRGB(Float3 const& image, int32_t begin, int32_t end) {
    for (int32_t i = begin; i < end; ++i) {
        float3 const color = spectrum::linear_RGB_to_sRGB(float3(image.at(i)));

        rgb_[i] = ::encoding::float_to_unorm(color);
    }
}

void Srgb::to_sRGB(Float4 const& image, int32_t begin, int32_t end) {
    if (dither_) {
        rnd::Generator rng(0, begin);

        for (int32_t i = begin; i < end; ++i) {
            float3 const color = spectrum::linear_RGB_to_sRGB(image.at(i).xyz());

            rgb_[i] = ::encoding::float_to_unorm(color, rng.random_float());
        }
    } else {
        for (int32_t i = begin; i < end; ++i) {
            float3 const color = spectrum::linear_RGB_to_sRGB(image.at(i).xyz());

            rgb_[i] = ::encoding::float_to_unorm(color);
        }
    }
}

void Srgb::to_byte(Float3 const& image, int32_t begin, int32_t end) {
    for (int32_t i = begin; i < end; ++i) {
        float3 const color = float3(image.at(i));

        rgb_[i] = ::encoding::float_to_unorm(color);
    }
}

void Srgb::to_byte(Float4 const& image, int32_t begin, int32_t end) {
    for (int32_t i = begin; i < end; ++i) {
        float3 const color = image.at(i).xyz();

        rgb_[i] = ::encoding::float_to_unorm(color);
    }
}

Srgb_alpha::Srgb_alpha(int2 dimensions) : rgba_(new byte4[(dimensions[0] * dimensions[1])]) {}

Srgb_alpha::~Srgb_alpha() {
    delete[] rgba_;
}

const byte4* Srgb_alpha::data() const {
    return rgba_;
}

void Srgb_alpha::to_sRGB(Float3 const& image, int32_t begin, int32_t end) {
    for (int32_t i = begin; i < end; ++i) {
        float4 color = float4(image.at(i), 1.f);
        color        = spectrum::linear_RGB_to_sRGB(color);

        rgba_[i] = ::encoding::float_to_unorm(color);
    }
}

void Srgb_alpha::to_sRGB(Float4 const& image, int32_t begin, int32_t end) {
    for (int32_t i = begin; i < end; ++i) {
        float4 color = image.at(i);
        color        = spectrum::linear_RGB_to_sRGB(color);

        rgba_[i] = ::encoding::float_to_unorm(color);
    }
}

void Srgb_alpha::to_byte(Float3 const& image, int32_t begin, int32_t end) {
    for (int32_t i = begin; i < end; ++i) {
        float4 const color = float4(image.at(i), 1.f);

        rgba_[i] = ::encoding::float_to_unorm(color);
    }
}

void Srgb_alpha::to_byte(Float4 const& image, int32_t begin, int32_t end) {
    for (int32_t i = begin; i < end; ++i) {
        float4 const color = image.at(i);

        rgba_[i] = ::encoding::float_to_unorm(color);
    }
}

}  // namespace image::encoding
