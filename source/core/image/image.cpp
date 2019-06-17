#ifndef SU_CORE_IMAGE_IMAGE_INL
#define SU_CORE_IMAGE_IMAGE_INL

#include "image.hpp"
#include "base/math/vector3.inl"

namespace image {

#define IMAGE_CONSTRUCTOR(TYPE, MEMBER) \
    Image::Image(TYPE&& image) noexcept : type_(Type::TYPE), MEMBER(std::move(image)) {}

#define IMAGE_DELEGATE(NAME, ...)                    \
    switch (type_) {                                 \
        case Type::Byte1:                            \
            return byte1_.NAME(__VA_ARGS__);         \
        case Type::Byte2:                            \
            return byte2_.NAME(__VA_ARGS__);         \
        case Type::Byte3:                            \
            return byte3_.NAME(__VA_ARGS__);         \
        case Type::Byte4:                            \
            return byte4_.NAME(__VA_ARGS__);         \
        case Type::Float1:                           \
            return float1_.NAME(__VA_ARGS__);        \
        case Type::Float1_sparse:                    \
            return float1_sparse_.NAME(__VA_ARGS__); \
        case Type::Float2:                           \
            return float2_.NAME(__VA_ARGS__);        \
        case Type::Float3:                           \
            return float3_.NAME(__VA_ARGS__);        \
        case Type::Float4:                           \
            return float4_.NAME(__VA_ARGS__);        \
    }

std::string Image::identifier() noexcept {
    return "Image";
}

IMAGE_CONSTRUCTOR(Byte1, byte1_)
IMAGE_CONSTRUCTOR(Byte2, byte2_)
IMAGE_CONSTRUCTOR(Byte3, byte3_)
IMAGE_CONSTRUCTOR(Byte4, byte4_)
IMAGE_CONSTRUCTOR(Float1, float1_)
IMAGE_CONSTRUCTOR(Float1_sparse, float1_sparse_)
IMAGE_CONSTRUCTOR(Float2, float2_)
IMAGE_CONSTRUCTOR(Float3, float3_)
IMAGE_CONSTRUCTOR(Float4, float4_)

Image::~Image() {
    switch (type_) {
        case Type::Byte1:
            byte1_.~Byte1();
            break;
        case Type::Byte2:
            byte2_.~Byte2();
            break;
        case Type::Byte3:
            byte3_.~Byte3();
            break;
        case Type::Byte4:
            byte4_.~Byte4();
            break;
        case Type::Float1:
            float1_.~Float1();
            break;
        case Type::Float1_sparse:
            float1_sparse_.~Float1_sparse();
            break;
        case Type::Float2:
            float2_.~Float2();
            break;
        case Type::Float3:
            float3_.~Float3();
            break;
        case Type::Float4:
            float4_.~Float4();
            break;
    }
}

Image::Type Image::type() const noexcept {
    return type_;
}

Description const& Image::description() const noexcept {
    IMAGE_DELEGATE(description)

    return byte1_.description();
}

Byte1 const& Image::byte1() const noexcept {
    return byte1_;
}

Byte2 const& Image::byte2() const noexcept {
    return byte2_;
}

Byte3 const& Image::byte3() const noexcept {
    return byte3_;
}

Byte4 const& Image::byte4() const noexcept {
    return byte4_;
}

Float1 const& Image::float1() const noexcept {
    return float1_;
}

Float1_sparse const& Image::float1_sparse() const noexcept {
    return float1_sparse_;
}

Float2 const& Image::float2() const noexcept {
    return float2_;
}

Float3 const& Image::float3() const noexcept {
    return float3_;
}

Float4 const& Image::float4() const noexcept {
    return float4_;
}

Byte1& Image::byte1() noexcept {
    return byte1_;
}

Byte2& Image::byte2() noexcept {
    return byte2_;
}

Byte3& Image::byte3() noexcept {
    return byte3_;
}

Byte4& Image::byte4() noexcept {
    return byte4_;
}

Float1& Image::float1() noexcept {
    return float1_;
}

Float1_sparse& Image::float1_sparse() noexcept {
    return float1_sparse_;
}

Float2& Image::float2() noexcept {
    return float2_;
}

Float3& Image::float3() noexcept {
    return float3_;
}

Float4& Image::float4() noexcept {
    return float4_;
}

size_t Image::num_bytes() const noexcept {
    IMAGE_DELEGATE(num_bytes)

    return 0;
}

}  // namespace image

#endif
