#include "encoding_srgb.hpp"
#include "base/encoding/encoding.inl"
#include "base/math/vector3.inl"
#include "base/spectrum/rgb.hpp"
#include "image/typed_image.hpp"

namespace image::encoding {

Srgb::Srgb(bool error_diffusion, bool alpha, bool pre_multiplied_alpha)
    : buffer_(nullptr),
      num_bytes_(0),
      error_diffusion_(error_diffusion),
      alpha_(alpha),
      pre_multiplied_alpha_(pre_multiplied_alpha) {}

Srgb::~Srgb() {
    delete[] buffer_;
}

bool Srgb::alpha() const {
    return alpha_;
}

void Srgb::resize(uint32_t num_pixels) {
    uint32_t const num_bytes = num_pixels * (alpha_ ? sizeof(byte4) : sizeof(byte3));

    if (num_bytes > num_bytes_) {
        delete[] buffer_;

        buffer_ = new char[num_bytes];

        num_bytes_ = num_bytes;
    }
}

static inline float golden_ratio(int32_t n) {
    return frac(float(n) * 0.618033988749894f);
}

char* Srgb::data() {
    return buffer_;
}

void Srgb::to_sRGB(Float4 const& image, int32_t begin, int32_t end) {
    int2 const d = image.description().dimensions().xy();

    if (alpha_) {
        byte4* rgba = reinterpret_cast<byte4*>(buffer_);

        if (error_diffusion_) {
            for (int32_t y = begin, i = begin * d[0]; y < end; ++y) {
                float4 error(golden_ratio(y) - 0.5f);

                for (int32_t x = 0; x < d[0]; ++x, ++i) {
                    float4 linear = image.at(i);

                    linear[3] = std::min(linear[3], 1.f);

                    if (!pre_multiplied_alpha_ && linear[3] > 0.f) {
                        linear = float4(linear.xyz() / linear[3], linear[3]);
                    }

                    float4 const color = spectrum::linear_to_gamma_sRGB(linear);

                    float4 const cf = 255.f * color;

                    byte4 const ci = byte4(cf + error + 0.5f);

                    error += cf - float4(ci);

                    rgba[i] = ci;
                }
            }
        } else {
            for (int32_t i = begin * d[0], len = end * d[0]; i < len; ++i) {
                float4 linear = image.at(i);

                linear[3] = std::min(linear[3], 1.f);

                if (!pre_multiplied_alpha_ && linear[3] > 0.f) {
                    linear = float4(linear.xyz() / linear[3], linear[3]);
                }

                float4 const color = spectrum::linear_to_gamma_sRGB(linear);

                rgba[i] = ::encoding::float_to_unorm(color);
            }
        }
    } else {
        byte3* rgb = reinterpret_cast<byte3*>(buffer_);

        if (error_diffusion_) {
            for (int32_t y = begin, i = begin * d[0]; y < end; ++y) {
                float3 error(golden_ratio(y) - 0.5f);

                for (int32_t x = 0; x < d[0]; ++x, ++i) {
                    float3 const color = spectrum::linear_to_gamma_sRGB(image.at(i).xyz());

                    float3 const cf = 255.f * color;

                    byte3 const ci = byte3(cf + error + 0.5f);

                    error += cf - float3(ci);

                    rgb[i] = ci;
                }
            }
        } else {
            for (int32_t i = begin * d[0], len = end * d[0]; i < len; ++i) {
                float3 const color = spectrum::linear_to_gamma_sRGB(image.at(i).xyz());

                rgb[i] = ::encoding::float_to_unorm(color);
            }
        }
    }
}

}  // namespace image::encoding
