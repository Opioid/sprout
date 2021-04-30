#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_INL
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_INL

#include "base/math/vector3.inl"
#include "base/math/vector4.inl"
#include "image/image.hpp"
#include "image/typed_image.hpp"
#include "scene/scene.inl"
#include "texture.hpp"
#include "texture_sampler.hpp"

namespace image::texture {

inline Texture::Texture() : type_(Type::Byte3_sRGB), image_(0xFFFFFFFF) {}

inline Texture::Texture(Type type, uint32_t image) : type_(type), image_(image), scale_(1.f) {}

inline bool Texture::operator==(Texture const& other) const {
    return type_ == other.type_ && image_ == other.image_ && scale_ == other.scale_;
}

inline bool Texture::is_valid() const {
    return 0xFFFFFFFF != image_;
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

inline Description const& Texture::description(Scene const& scene) const {
    return scene.image(image_)->description();
}

inline float2 Texture::scale() const {
    return scale_;
}

}  // namespace image::texture

#endif
