#include "encoding_srgb.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/vector3.inl"
#include "base/spectrum/rgb.hpp"
#include "image/typed_image.hpp"

namespace image::encoding {

Srgb::Srgb(bool error_diffusion) noexcept
    : rgb_(nullptr), num_pixels_(0), error_diffusion_(error_diffusion) {}

Srgb::~Srgb() noexcept {
    delete[] rgb_;
}

void Srgb::resize(uint32_t num_pixels) noexcept {
    if (num_pixels > num_pixels_) {
        delete[] rgb_;

        rgb_ = new byte3[num_pixels];

        num_pixels_ = num_pixels;
    }
}

byte3 const* Srgb::data() const noexcept {
    return rgb_;
}

static inline float golden_ratio(int32_t n) noexcept {
    return frac(float(n) * 0.618033988749894f);
}

void Srgb::to_sRGB(Float4 const& image, int32_t begin, int32_t end) noexcept {
    int2 const d = image.description().dimensions_2();

    if (error_diffusion_) {
        for (int32_t y = begin, i = begin * d[0]; y < end; ++y) {
            float3 error(golden_ratio(y) - 0.5f);

            for (int32_t x = 0; x < d[0]; ++x, ++i) {
                float3 const color = spectrum::linear_to_gamma_sRGB(image.at(i).xyz());

                float3 const cf = 255.f * color;
                byte3 const  ci = byte3(cf + error + 0.5f);

                error += cf - float3(ci);

                rgb_[i] = ci;
            }
        }
    } else {
        for (int32_t i = begin * d[0], len = end * d[0]; i < len; ++i) {
            float3 const color = spectrum::linear_to_gamma_sRGB(image.at(i).xyz());

            rgb_[i] = ::encoding::float_to_unorm(color);
        }
    }
}

Srgb_alpha::Srgb_alpha(bool error_diffusion, bool pre_multiplied_alpha) noexcept
    : rgba_(nullptr),
      num_pixels_(0),
      error_diffusion_(error_diffusion),
      pre_multiplied_alpha_(pre_multiplied_alpha) {}

Srgb_alpha::~Srgb_alpha() noexcept {
    delete[] rgba_;
}

void Srgb_alpha::resize(uint32_t num_pixels) noexcept {
    if (num_pixels > num_pixels_) {
        delete[] rgba_;

        rgba_ = new byte4[num_pixels];

        num_pixels_ = num_pixels;
    }
}

const byte4* Srgb_alpha::data() const noexcept {
    return rgba_;
}

void Srgb_alpha::to_sRGB(Float4 const& image, int32_t begin, int32_t end) noexcept {
    int2 const d = image.description().dimensions_2();

    if (error_diffusion_) {
        for (int32_t y = begin, i = begin * d[0]; y < end; ++y) {
            float4 error(golden_ratio(y) - 0.5f);

            for (int32_t x = 0; x < d[0]; ++x, ++i) {
                float4 linear = image.at(i);

                if (!pre_multiplied_alpha_ && linear[3] > 0.f) {
                    linear = float4(linear.xyz() / linear[3], linear[3]);
                }

                float4 const color = spectrum::linear_to_gamma_sRGB(linear);

                float4 const cf = 255.f * color;
                byte4 const  ci = byte4(cf + error + 0.5f);

                error += cf - float4(ci);

                rgba_[i] = ci;
            }
        }
    } else {
        for (int32_t i = begin * d[0], len = end * d[0]; i < len; ++i) {
            float4 linear = image.at(i);

            if (!pre_multiplied_alpha_ && linear[3] > 0.f) {
                linear = float4(linear.xyz() / linear[3], linear[3]);
            }

            float4 const color = spectrum::linear_to_gamma_sRGB(linear);

            rgba_[i] = ::encoding::float_to_unorm(color);
        }
    }
}

}  // namespace image::encoding
