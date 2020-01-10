#include "texture.hpp"
#include "texture.inl"

namespace image::texture {

char const* Texture::identifier() noexcept {
    return "Texture";
}

float Texture::average_1() const noexcept {
    float average(0.f);

    auto const& d = dimensions_3();

    for (int32_t i = 0, len = d[0] * d[1] * d[2]; i < len; ++i) {
        average += at_1(i);
    }

    auto const df = dimensions_float3();
    return average / (df[0] * df[1] * d[2]);
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

}  // namespace image::texture
