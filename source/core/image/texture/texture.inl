#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_INL
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_INL

#include "base/math/vector3.inl"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"
#include "texture.hpp"
#include "scene/scene.inl"
#include "image/image.hpp"
#include "texture_sampler.hpp"
#include "base/spectrum/aces.hpp"
#include "texture_encoding.inl"

namespace image::texture {

#define TEXTURE_CONSTRUCTOR(TYPE, MEMBER) \
    inline Texture::Texture(TYPE const& texture, uint32_t image_id) : type_(Type::TYPE), MEMBER(texture), image_id_(image_id) {}

TEXTURE_CONSTRUCTOR(Byte1_unorm, byte1_unorm_)
TEXTURE_CONSTRUCTOR(Byte2_snorm, byte2_snorm_)
TEXTURE_CONSTRUCTOR(Byte2_unorm, byte2_unorm_)
TEXTURE_CONSTRUCTOR(Byte3_unorm, byte3_unorm_)
TEXTURE_CONSTRUCTOR(Byte3_snorm, byte3_snorm_)
TEXTURE_CONSTRUCTOR(Byte3_sRGB, byte3_srgb_)
TEXTURE_CONSTRUCTOR(Byte4_sRGB, byte4_srgb_)
TEXTURE_CONSTRUCTOR(Half3, half3_)
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
        case Type::Half3:                            \
            return half3_.NAME(__VA_ARGS__);         \
        case Type::Float1:                           \
            return float1_.NAME(__VA_ARGS__);        \
        case Type::Float1_sparse:                    \
            return float1_sparse_.NAME(__VA_ARGS__); \
        case Type::Float2:                           \
            return float2_.NAME(__VA_ARGS__);        \
        case Type::Float3:                           \
            return float3_.NAME(__VA_ARGS__);        \
    }

inline int32_t Texture::num_channels() const {
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
        case Type::Half3:
        case Type::Float3:
            return 3;
        case Type::Byte4_sRGB:
            return 4;
    }
}

inline int32_t Texture::num_elements() const {
    TEXTURE_DELEGATE(description().num_elements)
}

inline int3 Texture::dimensions() const {
    TEXTURE_DELEGATE(dimensions)
}

inline int3 Texture::offset() const {
    TEXTURE_DELEGATE(description().offset)
}

inline float Texture::at_1(int32_t x, int32_t y) const {
    TEXTURE_DELEGATE(at_1, x, y)
}

inline float2 Texture::at_2(int32_t x, int32_t y) const {
    TEXTURE_DELEGATE(at_2, x, y)
}

inline float3 Texture::at_3(int32_t x, int32_t y) const {
    TEXTURE_DELEGATE(at_3, x, y)
}

inline float4 Texture::at_4(int32_t x, int32_t y) const {
    TEXTURE_DELEGATE(at_4, x, y)
}

inline void Texture::gather_1(int4_p xy_xy1, float c[4]) const {
    TEXTURE_DELEGATE(gather_1, xy_xy1, c)
}

inline void Texture::gather_2(int4_p xy_xy1, float2 c[4]) const {
    TEXTURE_DELEGATE(gather_2, xy_xy1, c)
}

inline void Texture::gather_3(int4_p xy_xy1, float3 c[4]) const {
    TEXTURE_DELEGATE(gather_3, xy_xy1, c)
}

inline float Texture::at_element_1(int32_t x, int32_t y, int32_t element) const {
    TEXTURE_DELEGATE(at_element_1, x, y, element)
}

inline float2 Texture::at_element_2(int32_t x, int32_t y, int32_t element) const {
    TEXTURE_DELEGATE(at_element_2, x, y, element)
}

inline float3 Texture::at_element_3(int32_t x, int32_t y, int32_t element) const {
    TEXTURE_DELEGATE(at_element_3, x, y, element)
}

inline float Texture::at_1(int32_t x, int32_t y, int32_t z) const {
    TEXTURE_DELEGATE(at_1, x, y, z)
}

inline float2 Texture::at_2(int32_t x, int32_t y, int32_t z) const {
    TEXTURE_DELEGATE(at_2, x, y, z)
}

inline float3 Texture::at_3(int32_t x, int32_t y, int32_t z) const {
    TEXTURE_DELEGATE(at_3, x, y, z)
}

inline float4 Texture::at_4(int32_t x, int32_t y, int32_t z) const {
    TEXTURE_DELEGATE(at_4, x, y, z)
}

inline void Texture::gather_1(int3_p xyz, int3_p xyz1, float c[8]) const {
    TEXTURE_DELEGATE(gather_1, xyz, xyz1, c);
}

inline void Texture::gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const {
    TEXTURE_DELEGATE(gather_2, xyz, xyz1, c);
}

inline Turbotexture::Turbotexture() : type_(Type::Byte3_sRGB), image_id_(0xFFFFFFFF) {}

inline Turbotexture::Turbotexture(Type type, uint32_t image) : type_(type), image_id_(image), scale_(1.f) {}

inline Description const& Turbotexture::description(Scene const& scene) const {
    return scene.image(image_id_)->description();
}

inline float2 Turbotexture::scale() const {
    return scale_;
}

}  // namespace image::texture

#endif
