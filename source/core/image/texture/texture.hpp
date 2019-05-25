#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP

#include <string>
#include "base/math/vector3.hpp"
#include "texture_byte_1_unorm.hpp"
#include "texture_byte_2_snorm.hpp"
#include "texture_byte_2_unorm.hpp"
#include "texture_byte_3_snorm.hpp"
#include "texture_byte_3_srgb.hpp"
#include "texture_byte_3_unorm.hpp"
#include "texture_byte_4_srgb.hpp"
#include "texture_float_1.hpp"
#include "texture_float_2.hpp"
#include "texture_float_3.hpp"

namespace image::texture {

class alignas(64) Texture {
  public:
    static std::string identifier() noexcept;

    enum class Type {
        Byte1_unorm,
        Byte2_snorm,
        Byte2_unorm,
        Byte3_snorm,
        Byte3_unorm,
        Byte3_sRGB,
        Byte4_sRGB,
        Float1,
        Float1_sparse,
        Float2,
        Float3,
    };

    Texture(Byte1_unorm const& texture) noexcept;
    Texture(Byte2_snorm const& texture) noexcept;
    Texture(Byte2_unorm const& texture) noexcept;
    Texture(Byte3_snorm const& texture) noexcept;
    Texture(Byte3_unorm const& texture) noexcept;
    Texture(Byte3_sRGB const& texture) noexcept;
    Texture(Byte4_sRGB const& texture) noexcept;
    Texture(Float1 const& texture) noexcept;
    Texture(Float1_sparse const& texture) noexcept;
    Texture(Float2 const& texture) noexcept;
    Texture(Float3 const& texture) noexcept;

    int32_t num_channels() const noexcept;
    int32_t num_elements() const noexcept;

    int2        dimensions_2() const noexcept;
    int3 const& dimensions_3() const noexcept;

    int2        back_2() const noexcept;
    int3 const& back_3() const noexcept;

    float2        dimensions_float2() const noexcept;
    float3 const& dimensions_float3() const noexcept;

    float  at_1(int32_t i) const noexcept;
    float3 at_3(int32_t i) const noexcept;

    float  at_1(int32_t x, int32_t y) const noexcept;
    float2 at_2(int32_t x, int32_t y) const noexcept;
    float3 at_3(int32_t x, int32_t y) const noexcept;

    void gather_1(int4 const& xy_xy1, float c[4]) const noexcept;
    void gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept;
    void gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept;

    float  at_element_1(int32_t x, int32_t y, int32_t element) const noexcept;
    float2 at_element_2(int32_t x, int32_t y, int32_t element) const noexcept;
    float3 at_element_3(int32_t x, int32_t y, int32_t element) const noexcept;

    float  at_1(int32_t x, int32_t y, int32_t z) const noexcept;
    float2 at_2(int32_t x, int32_t y, int32_t z) const noexcept;
    float3 at_3(int32_t x, int32_t y, int32_t z) const noexcept;
    float4 at_4(int32_t x, int32_t y, int32_t z) const noexcept;

    float average_1() const noexcept;

    float3 average_3() const noexcept;
    float3 average_3(int32_t element) const noexcept;

    size_t image_num_bytes() const noexcept;

  private:
    Type const type_;

    int3 const back_;

    float3 const dimensions_float_;

    union {
        Byte1_unorm const   byte1_unorm_;
        Byte2_snorm const   byte2_snorm_;
        Byte2_unorm const   byte2_unorm_;
        Byte3_snorm const   byte3_snorm_;
        Byte3_unorm const   byte3_unorm_;
        Byte3_sRGB const    byte3_srgb_;
        Byte4_sRGB const    byte4_srgb_;
        Float1 const        float1_;
        Float1_sparse const float1_sparse_;
        Float2 const        float2_;
        Float3 const        float3_;
    };
};

}  // namespace image::texture

#endif
