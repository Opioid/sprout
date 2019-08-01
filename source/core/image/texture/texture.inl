#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_INL
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_INL

#include "base/math/vector3.inl"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"
#include "texture.hpp"

namespace image::texture {

inline std::string Texture::identifier() noexcept {
    return "Texture";
}

#define TEXTURE_CONSTRUCTOR(TYPE, MEMBER)                              \
    inline Texture::Texture(TYPE const& texture) noexcept              \
        : type_(Type::TYPE),                                           \
          back_(texture.image().description().dimensions - 1),         \
          dimensions_float_(texture.image().description().dimensions), \
          MEMBER(texture) {}

TEXTURE_CONSTRUCTOR(Byte1_unorm, byte1_unorm_)
TEXTURE_CONSTRUCTOR(Byte2_snorm, byte2_snorm_)
TEXTURE_CONSTRUCTOR(Byte2_unorm, byte2_unorm_)
TEXTURE_CONSTRUCTOR(Byte3_unorm, byte3_unorm_)
TEXTURE_CONSTRUCTOR(Byte3_snorm, byte3_snorm_)
TEXTURE_CONSTRUCTOR(Byte3_sRGB, byte3_srgb_)
TEXTURE_CONSTRUCTOR(Float1, float1_)
TEXTURE_CONSTRUCTOR(Float1_sparse, float1_sparse_)
TEXTURE_CONSTRUCTOR(Float2, float2_)
TEXTURE_CONSTRUCTOR(Float3, float3_)

#define TEXTURE_DELEGATE(NAME, ...)                  \
    switch (type_) {                                 \
        case Type::Byte1_unorm:                      \
            return byte1_unorm_.NAME(__VA_ARGS__);   \
        case Type::Byte2_snorm:                      \
            return byte2_snorm_.NAME(__VA_ARGS__);   \
        case Type::Byte2_unorm:                      \
            return byte2_unorm_.NAME(__VA_ARGS__);   \
        case Type::Byte3_snorm:                      \
            return byte3_snorm_.NAME(__VA_ARGS__);   \
        case Type::Byte3_unorm:                      \
            return byte3_unorm_.NAME(__VA_ARGS__);   \
        case Type::Byte3_sRGB:                       \
            return byte3_srgb_.NAME(__VA_ARGS__);    \
        case Type::Byte4_sRGB:                       \
            return byte4_srgb_.NAME(__VA_ARGS__);    \
        case Type::Float1:                           \
            return float1_.NAME(__VA_ARGS__);        \
        case Type::Float1_sparse:                    \
            return float1_sparse_.NAME(__VA_ARGS__); \
        case Type::Float2:                           \
            return float2_.NAME(__VA_ARGS__);        \
        case Type::Float3:                           \
            return float3_.NAME(__VA_ARGS__);        \
    }

inline int32_t Texture::num_channels() const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
        case Type::Float1:
        case Type::Float1_sparse:
            return 1;
        case Type::Byte2_snorm:
        case Type::Byte2_unorm:
        case Type::Float2:
            return 2;
        case Type::Byte3_snorm:
        case Type::Byte3_unorm:
        case Type::Byte3_sRGB:
        case Type::Float3:
            return 3;
        case Type::Byte4_sRGB:
            return 4;
    }

    return 0;
}

inline int32_t Texture::num_elements() const noexcept {
    TEXTURE_DELEGATE(num_elements)

    return 0;
}

inline int2 Texture::dimensions_2() const noexcept {
    TEXTURE_DELEGATE(dimensions_2)

    return int2(0);
}

inline int3 const& Texture::dimensions_3() const noexcept {
    TEXTURE_DELEGATE(dimensions_3)

    return byte1_unorm_.dimensions_3();
}

inline int2 Texture::back_2() const noexcept {
    return back_.xy();
}

inline int3 const& Texture::back_3() const noexcept {
    return back_;
}

inline float2 Texture::dimensions_float2() const noexcept {
    return dimensions_float_.xy();
}

inline float3 const& Texture::dimensions_float3() const noexcept {
    return dimensions_float_;
}

inline float Texture::at_1(int32_t i) const noexcept {
    TEXTURE_DELEGATE(at_1, i)

    return 0.f;
}

inline float3 Texture::at_3(int32_t i) const noexcept {
    TEXTURE_DELEGATE(at_3, i)

    return float3(0.f);
}

inline float Texture::at_1(int32_t x, int32_t y) const noexcept {
    TEXTURE_DELEGATE(at_1, x, y)

    return 0.f;
}

inline float2 Texture::at_2(int32_t x, int32_t y) const noexcept {
    TEXTURE_DELEGATE(at_2, x, y)

    return float2(0.f);
}

inline float3 Texture::at_3(int32_t x, int32_t y) const noexcept {
    TEXTURE_DELEGATE(at_3, x, y)

    return float3(0.f);
}

inline float4 Texture::at_4(int32_t x, int32_t y) const noexcept {
    TEXTURE_DELEGATE(at_4, x, y)

    return float4(0.f);
}

inline void Texture::gather_1(int4 const& xy_xy1, float c[4]) const noexcept {
    TEXTURE_DELEGATE(gather_1, xy_xy1, c)
}

inline void Texture::gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept {
    TEXTURE_DELEGATE(gather_2, xy_xy1, c)
}

inline void Texture::gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept {
    TEXTURE_DELEGATE(gather_3, xy_xy1, c)
}

inline float Texture::at_element_1(int32_t x, int32_t y, int32_t element) const noexcept {
    TEXTURE_DELEGATE(at_element_1, x, y, element)

    return 0.f;
}

inline float2 Texture::at_element_2(int32_t x, int32_t y, int32_t element) const noexcept {
    TEXTURE_DELEGATE(at_element_2, x, y, element)

    return float2(0.f);
}

inline float3 Texture::at_element_3(int32_t x, int32_t y, int32_t element) const noexcept {
    TEXTURE_DELEGATE(at_element_3, x, y, element)

    return float3(0.);
}

inline float Texture::at_1(int32_t x, int32_t y, int32_t z) const noexcept {
    TEXTURE_DELEGATE(at_1, x, y, z)

    return 0.f;
}

inline float2 Texture::at_2(int32_t x, int32_t y, int32_t z) const noexcept {
    TEXTURE_DELEGATE(at_2, x, y, z)

    return float2(0.);
}

inline float3 Texture::at_3(int32_t x, int32_t y, int32_t z) const noexcept {
    TEXTURE_DELEGATE(at_3, x, y, z)

    return float3(0.f);
}

inline float4 Texture::at_4(int32_t x, int32_t y, int32_t z) const noexcept {
    TEXTURE_DELEGATE(at_4, x, y, z)

    return float4(0.f);
}

inline float Texture::average_1() const noexcept {
    float average(0.f);

    auto const& d = dimensions_3();

    for (int32_t i = 0, len = d[0] * d[1] * d[2]; i < len; ++i) {
        average += at_1(i);
    }

    auto const df = dimensions_float3();
    return average / (df[0] * df[1] * d[2]);
}

inline float3 Texture::average_3() const noexcept {
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

inline float3 Texture::average_3(int32_t element) const noexcept {
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

#endif
