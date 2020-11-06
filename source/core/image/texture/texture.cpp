#include "texture.hpp"
#include "texture.inl"

namespace image::texture {

char const* Texture::identifier() {
    return "Texture";
}

float Texture::average_1() const {
    float average(0.f);

    auto const& d = dimensions();

    for (int32_t z = 0, depth = d[2]; z < depth; ++z) {
        for (int32_t y = 0, height = d[1]; y < height; ++y) {
            for (int32_t x = 0, width = d[0]; x < width; ++x) {
                average += at_1(x, y, z);
            }
        }
    }

    auto const df = float3(d);
    return average / (df[0] * df[1] * d[2]);
}

float3 Texture::average_3() const {
    float3 average(0.f);

    auto const d = dimensions().xy();
    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            average += at_3(x, y);
        }
    }

    auto const df = float2(d);
    return average / (df[0] * df[1]);
}

float3 Texture::average_3(int32_t element) const {
    float3 average(0.f);

    auto const d = dimensions().xy();
    for (int32_t y = 0; y < d[1]; ++y) {
        for (int32_t x = 0; x < d[0]; ++x) {
            average += at_element_3(x, y, element);
        }
    }

    auto const df = float2(d);
    return average / (df[0] * df[1]);
}

}  // namespace image::texture
