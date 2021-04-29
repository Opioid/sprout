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

inline Turbotexture::Turbotexture() : type_(Type::Byte3_sRGB), image_id_(0xFFFFFFFF) {}

inline Turbotexture::Turbotexture(Type type, uint32_t image) : type_(type), image_id_(image), scale_(1.f) {}

inline bool Turbotexture::operator==(Turbotexture const& other) const {
    return type_ == other.type_ && image_id_ == other.image_id_ && scale_ == other.scale_;
}

inline bool Turbotexture::is_valid() const {
    return 0xFFFFFFFF != image_id_;
}

inline int32_t Turbotexture::num_channels() const {
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

inline Description const& Turbotexture::description(Scene const& scene) const {
    return scene.image(image_id_)->description();
}

inline float2 Turbotexture::scale() const {
    return scale_;
}

}  // namespace image::texture

#endif
