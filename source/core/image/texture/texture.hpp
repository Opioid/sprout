#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP

#include "base/math/vector3.hpp"
#include "texture_byte1_unorm.hpp"
#include "texture_byte2_snorm.hpp"
#include "texture_byte2_unorm.hpp"
#include "texture_byte3_snorm.hpp"
#include "texture_byte3_srgb.hpp"
#include "texture_byte3_unorm.hpp"
#include "texture_byte4_srgb.hpp"
#include "texture_float1.hpp"
#include "texture_float2.hpp"
#include "texture_float3.hpp"
#include "texture_half3.hpp"

namespace image::texture {

class Texture {
  public:
    static char const* identifier();

    Texture(Byte1_unorm const& texture);
    Texture(Byte2_snorm const& texture);
    Texture(Byte2_unorm const& texture);
    Texture(Byte3_snorm const& texture);
    Texture(Byte3_unorm const& texture);
    Texture(Byte3_sRGB const& texture);
    Texture(Byte4_sRGB const& texture);
    Texture(Half3 const& texture);
    Texture(Float1 const& texture);
    Texture(Float1_sparse const& texture);
    Texture(Float2 const& texture);
    Texture(Float3 const& texture);

    int32_t num_channels() const;

    int32_t num_elements() const;

    int32_t volume() const;

    int3_p dimensions() const;

    int3_p offset() const;

    float  at_1(int32_t x, int32_t y) const;
    float2 at_2(int32_t x, int32_t y) const;
    float3 at_3(int32_t x, int32_t y) const;
    float4 at_4(int32_t x, int32_t y) const;

    void gather_1(int4_p xy_xy1, float c[4]) const;
    void gather_2(int4_p xy_xy1, float2 c[4]) const;
    void gather_3(int4_p xy_xy1, float3 c[4]) const;

    float  at_element_1(int32_t x, int32_t y, int32_t element) const;
    float2 at_element_2(int32_t x, int32_t y, int32_t element) const;
    float3 at_element_3(int32_t x, int32_t y, int32_t element) const;

    float  at_1(int32_t x, int32_t y, int32_t z) const;
    float2 at_2(int32_t x, int32_t y, int32_t z) const;
    float3 at_3(int32_t x, int32_t y, int32_t z) const;
    float4 at_4(int32_t x, int32_t y, int32_t z) const;

    void gather_1(int3 const& xyz, int3_p xyz1, float c[8]) const;
    void gather_2(int3 const& xyz, int3_p xyz1, float2 c[8]) const;

    float average_1() const;

    float3 average_3() const;

    float3 average_3(int32_t element) const;

  private:
    enum class Type {
        Byte1_unorm,
        Byte2_snorm,
        Byte2_unorm,
        Byte3_snorm,
        Byte3_unorm,
        Byte3_sRGB,
        Byte4_sRGB,
        Half3,
        Float1,
        Float1_sparse,
        Float2,
        Float3,
    };

    int3 const dimensions_;

    Type const type_;

    union {
        Byte1_unorm const   byte1_unorm_;
        Byte2_snorm const   byte2_snorm_;
        Byte2_unorm const   byte2_unorm_;
        Byte3_snorm const   byte3_snorm_;
        Byte3_unorm const   byte3_unorm_;
        Byte3_sRGB const    byte3_srgb_;
        Byte4_sRGB const    byte4_srgb_;
        Half3 const         half3_;
        Float1 const        float1_;
        Float1_sparse const float1_sparse_;
        Float2 const        float2_;
        Float3 const        float3_;
    };
};

}  // namespace image::texture

#endif
