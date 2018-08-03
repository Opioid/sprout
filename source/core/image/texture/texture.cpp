#include "texture.hpp"
#include "base/math/vector4.inl"
#include "image/image.hpp"

namespace image::texture {

Texture::Texture(std::shared_ptr<Image> const& image) noexcept
    : untyped_image_(image),
      back_(int3(image->description().dimensions[0] - 1, image->description().dimensions[1] - 1,
                 image->description().dimensions[2] - 1)),
      dimensions_float_(float3(image->description().dimensions)) {}

Texture::~Texture() noexcept {}

Image const& Texture::image() const noexcept {
    return *untyped_image_.get();
}

int32_t Texture::num_channels() const noexcept {
    return untyped_image_->description().num_channels();
}

int32_t Texture::num_elements() const noexcept {
    return untyped_image_->description().num_elements;
}

int2 Texture::dimensions_2() const noexcept {
    return untyped_image_->description().dimensions.xy();
}

int3 const& Texture::dimensions_3() const noexcept {
    return untyped_image_->description().dimensions;
}

int2 Texture::back_2() const noexcept {
    return back_.xy();
}

int3 const& Texture::back_3() const noexcept {
    return back_;
}

float2 Texture::dimensions_float2() const noexcept {
    return dimensions_float_.xy();
}

float3 const& Texture::dimensions_float3() const noexcept {
    return dimensions_float_;
}

float3 Texture::average_3() const noexcept {
    float3 average(0.f);

    auto const d = dimensions_2();
    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            average += at_3(x, y);
        }
    }

    auto const df = dimensions_float2();
    return average / (df[0] * df[1]);
}

float3 Texture::average_3(int32_t element) const noexcept {
    float3 average(0.f);

    auto const d = dimensions_2();
    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            average += at_element_3(x, y, element);
        }
    }

    auto const df = dimensions_float2();
    return average / (df[0] * df[1]);
}

float4 Texture::average_4() const noexcept {
    float4 average(0.f);

    auto const d = dimensions_2();
    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            average += at_4(x, y);
        }
    }

    auto const df = dimensions_float2();
    return average / (df[0] * df[1]);
}

}  // namespace image::texture
