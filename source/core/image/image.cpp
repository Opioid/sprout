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
        case Type::Short3:                           \
            return short3_.NAME(__VA_ARGS__);        \
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

char const* Image::identifier() {
    return "Image";
}

IMAGE_CONSTRUCTOR(Byte1, byte1_)
IMAGE_CONSTRUCTOR(Byte2, byte2_)
IMAGE_CONSTRUCTOR(Byte3, byte3_)
IMAGE_CONSTRUCTOR(Byte4, byte4_)
IMAGE_CONSTRUCTOR(Short3, short3_)
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
        case Type::Short3:
            short3_.~Short3();
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

Image::Type Image::type() const {
    return type_;
}

Description const& Image::description() const {
    IMAGE_DELEGATE(description)

    return byte1_.description();
}

char* Image::data() const {
    switch (type_) {
        case Type::Byte1:
            return reinterpret_cast<char*>(byte1_.data());
        case Type::Byte2:
            return reinterpret_cast<char*>(byte2_.data());
        case Type::Byte3:
            return reinterpret_cast<char*>(byte3_.data());
        case Type::Byte4:
            return reinterpret_cast<char*>(byte4_.data());
        case Type::Short3:
            return reinterpret_cast<char*>(short3_.data());
        case Type::Float1:
            return reinterpret_cast<char*>(float1_.data());
        case Type::Float1_sparse:
            return nullptr;
        case Type::Float2:
            return reinterpret_cast<char*>(float2_.data());
        case Type::Float3:
            return reinterpret_cast<char*>(float3_.data());
        case Type::Float4:
            return reinterpret_cast<char*>(float4_.data());
    }

    return reinterpret_cast<char*>(byte1_.data());
}

Byte1 const& Image::byte1() const {
    return byte1_;
}

Byte2 const& Image::byte2() const {
    return byte2_;
}

Byte3 const& Image::byte3() const {
    return byte3_;
}

Byte4 const& Image::byte4() const {
    return byte4_;
}

Short3 const& Image::short3() const {
    return short3_;
}

Float1 const& Image::float1() const {
    return float1_;
}

Float1_sparse const& Image::float1_sparse() const {
    return float1_sparse_;
}

Float2 const& Image::float2() const {
    return float2_;
}

Float3 const& Image::float3() const {
    return float3_;
}

Float4 const& Image::float4() const {
    return float4_;
}

Byte1& Image::byte1() {
    return byte1_;
}

Byte2& Image::byte2() {
    return byte2_;
}

Byte3& Image::byte3() {
    return byte3_;
}

Byte4& Image::byte4() {
    return byte4_;
}

Short3& Image::short3() {
    return short3_;
}

Float1& Image::float1() {
    return float1_;
}

Float1_sparse& Image::float1_sparse() {
    return float1_sparse_;
}

Float2& Image::float2() {
    return float2_;
}

Float3& Image::float3() {
    return float3_;
}

Float4& Image::float4() {
    return float4_;
}

}  // namespace image

#endif
