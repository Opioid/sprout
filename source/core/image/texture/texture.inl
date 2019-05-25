#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_INL
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_INL

#include "base/math/vector3.inl"
#include "base/math/vector4.inl"
#include "image/image.hpp"
#include "texture.hpp"

namespace image::texture {

inline std::string Texture::identifier() noexcept {
    return "Texture";
}

inline Texture::Texture(Byte1_unorm const& texture) noexcept
    : type_(Type::Byte1_unorm),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      byte1_unorm_(texture) {}

inline Texture::Texture(Byte2_snorm const& texture) noexcept
    : type_(Type::Byte2_snorm),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      byte2_snorm_(texture) {}

inline Texture::Texture(Byte2_unorm const& texture) noexcept
    : type_(Type::Byte2_unorm),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      byte2_unorm_(texture) {}

inline Texture::Texture(Byte3_snorm const& texture) noexcept
    : type_(Type::Byte3_snorm),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      byte3_snorm_(texture) {}

inline Texture::Texture(Byte3_unorm const& texture) noexcept
    : type_(Type::Byte3_unorm),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      byte3_unorm_(texture) {}

inline Texture::Texture(Byte3_sRGB const& texture) noexcept
    : type_(Type::Byte3_sRGB),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      byte3_srgb_(texture) {}

inline Texture::Texture(Byte4_sRGB const& texture) noexcept
    : type_(Type::Byte4_sRGB),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      byte4_srgb_(texture) {}

inline Texture::Texture(Float1 const& texture) noexcept
    : type_(Type::Float1),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      float1_(texture) {}

inline Texture::Texture(Float1_sparse const& texture) noexcept
    : type_(Type::Float1_sparse),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      float1_sparse_(texture) {}

inline Texture::Texture(Float2 const& texture) noexcept
    : type_(Type::Float2),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      float2_(texture) {}

inline Texture::Texture(Float3 const& texture) noexcept
    : type_(Type::Float3),
      back_(texture.image().description().dimensions - 1),
      dimensions_float_(texture.image().description().dimensions),
      float3_(texture) {}

inline int32_t Texture::num_channels() const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.num_channels();
        case Type::Byte2_snorm:
            return byte2_snorm_.num_channels();
        case Type::Byte2_unorm:
            return byte2_unorm_.num_channels();
        case Type::Byte3_snorm:
            return byte3_snorm_.num_channels();
        case Type::Byte3_unorm:
            return byte3_unorm_.num_channels();
        case Type::Byte3_sRGB:
            return byte3_srgb_.num_channels();
        case Type::Byte4_sRGB:
            return byte4_srgb_.num_channels();
        case Type::Float1:
            return float1_.num_channels();
        case Type::Float1_sparse:
            return float1_sparse_.num_channels();
        case Type::Float2:
            return float2_.num_channels();
        case Type::Float3:
            return float3_.num_channels();
    }

    return 0;
}

inline int32_t Texture::num_elements() const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.num_elements();
        case Type::Byte2_snorm:
            return byte2_snorm_.num_elements();
        case Type::Byte2_unorm:
            return byte2_unorm_.num_elements();
        case Type::Byte3_snorm:
            return byte3_snorm_.num_elements();
        case Type::Byte3_unorm:
            return byte3_unorm_.num_elements();
        case Type::Byte3_sRGB:
            return byte3_srgb_.num_elements();
        case Type::Byte4_sRGB:
            return byte4_srgb_.num_elements();
        case Type::Float1:
            return float1_.num_elements();
        case Type::Float1_sparse:
            return float1_sparse_.num_elements();
        case Type::Float2:
            return float2_.num_elements();
        case Type::Float3:
            return float3_.num_elements();
    }

    return 0;
}

inline int2 Texture::dimensions_2() const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.dimensions_2();
        case Type::Byte2_snorm:
            return byte2_snorm_.dimensions_2();
        case Type::Byte2_unorm:
            return byte2_unorm_.dimensions_2();
        case Type::Byte3_snorm:
            return byte3_snorm_.dimensions_2();
        case Type::Byte3_unorm:
            return byte3_unorm_.dimensions_2();
        case Type::Byte3_sRGB:
            return byte3_srgb_.dimensions_2();
        case Type::Byte4_sRGB:
            return byte4_srgb_.dimensions_2();
        case Type::Float1:
            return float1_.dimensions_2();
        case Type::Float1_sparse:
            return float1_sparse_.dimensions_2();
        case Type::Float2:
            return float2_.dimensions_2();
        case Type::Float3:
            return float3_.dimensions_2();
    }

    return int2(0);
}

inline int3 const& Texture::dimensions_3() const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.dimensions_3();
        case Type::Byte2_snorm:
            return byte2_snorm_.dimensions_3();
        case Type::Byte2_unorm:
            return byte2_unorm_.dimensions_3();
        case Type::Byte3_snorm:
            return byte3_snorm_.dimensions_3();
        case Type::Byte3_unorm:
            return byte3_unorm_.dimensions_3();
        case Type::Byte3_sRGB:
            return byte3_srgb_.dimensions_3();
        case Type::Byte4_sRGB:
            return byte4_srgb_.dimensions_3();
        case Type::Float1:
            return float1_.dimensions_3();
        case Type::Float1_sparse:
            return float1_sparse_.dimensions_3();
        case Type::Float2:
            return float2_.dimensions_3();
        case Type::Float3:
            return float3_.dimensions_3();
    }

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
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_1(i);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_1(i);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_1(i);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_1(i);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_1(i);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_1(i);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_1(i);
        case Type::Float1:
            return float1_.at_1(i);
        case Type::Float1_sparse:
            return float1_sparse_.at_1(i);
        case Type::Float2:
            return float2_.at_1(i);
        case Type::Float3:
            return float3_.at_1(i);
    }

    return 0.f;
}

inline float3 Texture::at_3(int32_t i) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_3(i);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_3(i);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_3(i);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_3(i);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_3(i);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_3(i);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_3(i);
        case Type::Float1:
            return float1_.at_3(i);
        case Type::Float1_sparse:
            return float1_sparse_.at_3(i);
        case Type::Float2:
            return float2_.at_3(i);
        case Type::Float3:
            return float3_.at_3(i);
    }

    return float3(0.f);
}

inline float Texture::at_1(int32_t x, int32_t y) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_1(x, y);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_1(x, y);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_1(x, y);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_1(x, y);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_1(x, y);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_1(x, y);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_1(x, y);
        case Type::Float1:
            return float1_.at_1(x, y);
        case Type::Float1_sparse:
            return float1_sparse_.at_1(x, y);
        case Type::Float2:
            return float2_.at_1(x, y);
        case Type::Float3:
            return float3_.at_1(x, y);
    }

    return 0.f;
}

inline float2 Texture::at_2(int32_t x, int32_t y) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_2(x, y);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_2(x, y);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_2(x, y);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_2(x, y);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_2(x, y);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_2(x, y);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_2(x, y);
        case Type::Float1:
            return float1_.at_2(x, y);
        case Type::Float1_sparse:
            return float1_sparse_.at_2(x, y);
        case Type::Float2:
            return float2_.at_2(x, y);
        case Type::Float3:
            return float3_.at_2(x, y);
    }

    return float2(0.f);
}

inline float3 Texture::at_3(int32_t x, int32_t y) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_3(x, y);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_3(x, y);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_3(x, y);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_3(x, y);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_3(x, y);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_3(x, y);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_3(x, y);
        case Type::Float1:
            return float1_.at_3(x, y);
        case Type::Float1_sparse:
            return float1_sparse_.at_3(x, y);
        case Type::Float2:
            return float2_.at_3(x, y);
        case Type::Float3:
            return float3_.at_3(x, y);
    }

    return float3(0.);
}

inline void Texture::gather_1(int4 const& xy_xy1, float c[4]) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.gather_1(xy_xy1, c);
        case Type::Byte2_snorm:
            return byte2_snorm_.gather_1(xy_xy1, c);
        case Type::Byte2_unorm:
            return byte2_unorm_.gather_1(xy_xy1, c);
        case Type::Byte3_snorm:
            return byte3_snorm_.gather_1(xy_xy1, c);
        case Type::Byte3_unorm:
            return byte3_unorm_.gather_1(xy_xy1, c);
        case Type::Byte3_sRGB:
            return byte3_srgb_.gather_1(xy_xy1, c);
        case Type::Byte4_sRGB:
            return byte4_srgb_.gather_1(xy_xy1, c);
        case Type::Float1:
            return float1_.gather_1(xy_xy1, c);
        case Type::Float1_sparse:
            return float1_sparse_.gather_1(xy_xy1, c);
        case Type::Float2:
            return float2_.gather_1(xy_xy1, c);
        case Type::Float3:
            return float3_.gather_1(xy_xy1, c);
    }
}

inline void Texture::gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.gather_2(xy_xy1, c);
        case Type::Byte2_snorm:
            return byte2_snorm_.gather_2(xy_xy1, c);
        case Type::Byte2_unorm:
            return byte2_unorm_.gather_2(xy_xy1, c);
        case Type::Byte3_snorm:
            return byte3_snorm_.gather_2(xy_xy1, c);
        case Type::Byte3_unorm:
            return byte3_unorm_.gather_2(xy_xy1, c);
        case Type::Byte3_sRGB:
            return byte3_srgb_.gather_2(xy_xy1, c);
        case Type::Byte4_sRGB:
            return byte4_srgb_.gather_2(xy_xy1, c);
        case Type::Float1:
            return float1_.gather_2(xy_xy1, c);
        case Type::Float1_sparse:
            return float1_sparse_.gather_2(xy_xy1, c);
        case Type::Float2:
            return float2_.gather_2(xy_xy1, c);
        case Type::Float3:
            return float3_.gather_2(xy_xy1, c);
    }
}

inline void Texture::gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.gather_3(xy_xy1, c);
        case Type::Byte2_snorm:
            return byte2_snorm_.gather_3(xy_xy1, c);
        case Type::Byte2_unorm:
            return byte2_unorm_.gather_3(xy_xy1, c);
        case Type::Byte3_snorm:
            return byte3_snorm_.gather_3(xy_xy1, c);
        case Type::Byte3_unorm:
            return byte3_unorm_.gather_3(xy_xy1, c);
        case Type::Byte3_sRGB:
            return byte3_srgb_.gather_3(xy_xy1, c);
        case Type::Byte4_sRGB:
            return byte4_srgb_.gather_3(xy_xy1, c);
        case Type::Float1:
            return float1_.gather_3(xy_xy1, c);
        case Type::Float1_sparse:
            return float1_sparse_.gather_3(xy_xy1, c);
        case Type::Float2:
            return float2_.gather_3(xy_xy1, c);
        case Type::Float3:
            return float3_.gather_3(xy_xy1, c);
    }
}

inline float Texture::at_element_1(int32_t x, int32_t y, int32_t element) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_element_1(x, y, element);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_element_1(x, y, element);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_element_1(x, y, element);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_element_1(x, y, element);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_element_1(x, y, element);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_element_1(x, y, element);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_element_1(x, y, element);
        case Type::Float1:
            return float1_.at_element_1(x, y, element);
        case Type::Float1_sparse:
            return float1_sparse_.at_element_1(x, y, element);
        case Type::Float2:
            return float2_.at_element_1(x, y, element);
        case Type::Float3:
            return float3_.at_element_1(x, y, element);
    }

    return 0.f;
}

inline float2 Texture::at_element_2(int32_t x, int32_t y, int32_t element) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_element_2(x, y, element);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_element_2(x, y, element);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_element_2(x, y, element);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_element_2(x, y, element);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_element_2(x, y, element);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_element_2(x, y, element);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_element_2(x, y, element);
        case Type::Float1:
            return float1_.at_element_2(x, y, element);
        case Type::Float1_sparse:
            return float1_sparse_.at_element_2(x, y, element);
        case Type::Float2:
            return float2_.at_element_2(x, y, element);
        case Type::Float3:
            return float3_.at_element_2(x, y, element);
    }

    return float2(0.f);
}

inline float3 Texture::at_element_3(int32_t x, int32_t y, int32_t element) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_element_3(x, y, element);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_element_3(x, y, element);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_element_3(x, y, element);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_element_3(x, y, element);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_element_3(x, y, element);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_element_3(x, y, element);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_element_3(x, y, element);
        case Type::Float1:
            return float1_.at_element_3(x, y, element);
        case Type::Float1_sparse:
            return float1_sparse_.at_element_3(x, y, element);
        case Type::Float2:
            return float2_.at_element_3(x, y, element);
        case Type::Float3:
            return float3_.at_element_3(x, y, element);
    }

    return float3(0.);
}

inline float Texture::at_1(int32_t x, int32_t y, int32_t z) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_1(x, y, z);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_1(x, y, z);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_1(x, y, z);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_1(x, y, z);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_1(x, y, z);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_1(x, y, z);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_1(x, y, z);
        case Type::Float1:
            return float1_.at_1(x, y, z);
        case Type::Float1_sparse:
            return float1_sparse_.at_1(x, y);
        case Type::Float2:
            return float2_.at_1(x, y, z);
        case Type::Float3:
            return float3_.at_1(x, y, z);
    }

    return 0.f;
}

inline float2 Texture::at_2(int32_t x, int32_t y, int32_t z) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_2(x, y, z);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_2(x, y, z);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_2(x, y, z);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_2(x, y, z);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_2(x, y, z);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_2(x, y, z);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_2(x, y, z);
        case Type::Float1:
            return float1_.at_2(x, y, z);
        case Type::Float1_sparse:
            return float1_sparse_.at_2(x, y, z);
        case Type::Float2:
            return float2_.at_2(x, y, z);
        case Type::Float3:
            return float3_.at_2(x, y, z);
    }

    return float2(0.);
}

inline float3 Texture::at_3(int32_t x, int32_t y, int32_t z) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_3(x, y, z);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_3(x, y, z);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_3(x, y, z);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_3(x, y, z);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_3(x, y, z);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_3(x, y, z);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_3(x, y, z);
        case Type::Float1:
            return float1_.at_3(x, y, z);
        case Type::Float1_sparse:
            return float1_sparse_.at_3(x, y, z);
        case Type::Float2:
            return float2_.at_3(x, y, z);
        case Type::Float3:
            return float3_.at_3(x, y, z);
    }

    return float3(0.f);
}

inline float4 Texture::at_4(int32_t x, int32_t y, int32_t z) const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.at_4(x, y, z);
        case Type::Byte2_snorm:
            return byte2_snorm_.at_4(x, y, z);
        case Type::Byte2_unorm:
            return byte2_unorm_.at_4(x, y, z);
        case Type::Byte3_snorm:
            return byte3_snorm_.at_4(x, y, z);
        case Type::Byte3_unorm:
            return byte3_unorm_.at_4(x, y, z);
        case Type::Byte3_sRGB:
            return byte3_srgb_.at_4(x, y, z);
        case Type::Byte4_sRGB:
            return byte4_srgb_.at_4(x, y, z);
        case Type::Float1:
            return float1_.at_4(x, y, z);
        case Type::Float1_sparse:
            return float1_sparse_.at_4(x, y, z);
        case Type::Float2:
            return float2_.at_4(x, y, z);
        case Type::Float3:
            return float3_.at_4(x, y, z);
    }

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

inline size_t Texture::image_num_bytes() const noexcept {
    switch (type_) {
        case Type::Byte1_unorm:
            return byte1_unorm_.image_num_bytes();
        case Type::Byte2_snorm:
            return byte2_snorm_.image_num_bytes();
        case Type::Byte2_unorm:
            return byte2_unorm_.image_num_bytes();
        case Type::Byte3_snorm:
            return byte3_snorm_.image_num_bytes();
        case Type::Byte3_unorm:
            return byte3_unorm_.image_num_bytes();
        case Type::Byte3_sRGB:
            return byte3_srgb_.image_num_bytes();
        case Type::Byte4_sRGB:
            return byte4_srgb_.image_num_bytes();
        case Type::Float1:
            return float1_.image_num_bytes();
        case Type::Float1_sparse:
            return float1_sparse_.image_num_bytes();
        case Type::Float2:
            return float2_.image_num_bytes();
        case Type::Float3:
            return float3_.image_num_bytes();
    }

    return 0;
}

}  // namespace image::texture

#endif
