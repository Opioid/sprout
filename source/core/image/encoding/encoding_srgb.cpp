#include "encoding_srgb.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/vector3.inl"
#include "base/spectrum/rgb.hpp"
#include "image/typed_image.inl"

namespace image::encoding {

Srgb::Srgb(int2 dimensions, bool error_diffusion)
    : rgb_(new byte3[static_cast<size_t>(dimensions[0] * dimensions[1])]),
      error_diffusion_(error_diffusion) {}

Srgb::~Srgb() {
    delete[] rgb_;
}

byte3 const* Srgb::data() const {
    return rgb_;
}

static inline float golden_ratio(uint32_t n) noexcept {
    return frac(static_cast<float>(n) * 0.618033988749894f);
}

void Srgb::to_sRGB(Float4 const& image, int32_t begin, int32_t end) {
    int2 const d = image.dimensions2();

    if (error_diffusion_) {
        for (int32_t y = begin, i = begin * d[0]; y < end; ++y) {
            float3 error(golden_ratio(y) - 0.5f);
            for (int32_t x = 0; x < d[0]; ++x, ++i) {
                float3 const color = spectrum::linear_RGB_to_sRGB(image.at(i).xyz());

                float3 const co = 255.f * color;
                float3 const cf = co + error + 0.5f;
                byte3 const  ci = byte3(cf);

                error += co - float3(ci);

                rgb_[i] = ci;
            }
        }

    } else {
        for (int32_t i = begin * d[0], len = end * d[0]; i < len; ++i) {
            float3 const color = spectrum::linear_RGB_to_sRGB(image.at(i).xyz());

            rgb_[i] = ::encoding::float_to_unorm(color);
        }
    }
}

Srgb_alpha::Srgb_alpha(int2 dimensions, bool error_diffusion)
    : rgba_(new byte4[(dimensions[0] * dimensions[1])]), error_diffusion_(error_diffusion) {}

Srgb_alpha::~Srgb_alpha() {
    delete[] rgba_;
}

const byte4* Srgb_alpha::data() const {
    return rgba_;
}

void Srgb_alpha::to_sRGB(Float4 const& image, int32_t begin, int32_t end) {
    int2 const d = image.dimensions2();

    if (error_diffusion_) {
        for (int32_t y = begin, i = begin * d[0]; y < end; ++y) {
            float4 error(golden_ratio(y) - 0.5f);
            for (int32_t x = 0; x < d[0]; ++x, ++i) {
                float4 const color = spectrum::linear_RGB_to_sRGB(image.at(i));

                float4 const co = 255.f * color;
                float4 const cf = co + error + 0.5f;
                byte4 const  ci = byte4(cf);

                error += co - float4(ci);

                rgba_[i] = ci;
            }
        }

    } else {
        for (int32_t i = begin * d[0], len = end * d[0]; i < len; ++i) {
            float4 const color = spectrum::linear_RGB_to_sRGB(image.at(i));

            rgba_[i] = ::encoding::float_to_unorm(color);
        }
    }
}

}  // namespace image::encoding
