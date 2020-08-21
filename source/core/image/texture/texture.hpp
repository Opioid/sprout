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

class Sampler_2D;
class Sampler_3D;


class alignas(16) Texture {
  public:
    static char const* identifier();

    Texture();
    Texture(float scale, Byte1_unorm const& texture);
    Texture(float scale, Byte2_snorm const& texture);
    Texture(float scale, Byte2_unorm const& texture);
    Texture(float scale, Byte3_snorm const& texture);
    Texture(float scale, Byte3_unorm const& texture);
    Texture(float scale, Byte3_sRGB const& texture);
    Texture(float scale, Byte4_sRGB const& texture);
    Texture(float scale, Half3 const& texture);
    Texture(float scale, Float1 const& texture);
    Texture(float scale, Float1_sparse const& texture);
    Texture(float scale, Float2 const& texture);
    Texture(float scale, Float3 const& texture);

    bool is_valid() const;

    bool operator==(Texture const& other) const;

    int32_t num_channels() const;

    int32_t num_elements() const;

    int32_t volume() const;

    int3 const& dimensions() const;

    int3 const& offset() const;

    float  at_1(int32_t x, int32_t y) const;
    float2 at_2(int32_t x, int32_t y) const;
    float3 at_3(int32_t x, int32_t y) const;
    float4 at_4(int32_t x, int32_t y) const;

    void gather_1(int4 const& xy_xy1, float c[4]) const;
    void gather_2(int4 const& xy_xy1, float2 c[4]) const;
    void gather_3(int4 const& xy_xy1, float3 c[4]) const;

    float  at_element_1(int32_t x, int32_t y, int32_t element) const;
    float2 at_element_2(int32_t x, int32_t y, int32_t element) const;
    float3 at_element_3(int32_t x, int32_t y, int32_t element) const;

    float  at_1(int32_t x, int32_t y, int32_t z) const;
    float2 at_2(int32_t x, int32_t y, int32_t z) const;
    float3 at_3(int32_t x, int32_t y, int32_t z) const;
    float4 at_4(int32_t x, int32_t y, int32_t z) const;

    void gather_1(int3 const& xyz, int3 const& xyz1, float c[8]) const;
    void gather_2(int3 const& xyz, int3 const& xyz1, float2 c[8]) const;

    float average_1() const;

    float3 average_3() const;

    float3 average_3(int32_t element) const;

    float  sample_1(Sampler_2D const& sampler, float2 uv) const;
        float2 sample_2(Sampler_2D const& sampler, float2 uv) const;
        float3 sample_3(Sampler_2D const& sampler, float2 uv) const;

        float sample_1(Sampler_2D const& sampler, float2 uv,
                       int32_t element) const;

        float2 sample_2(Sampler_2D const& sampler, float2 uv,
                        int32_t element) const;

        float3 sample_3(Sampler_2D const& sampler, float2 uv,
                        int32_t element) const;

        float2 address(Sampler_2D const& sampler, float2 uv) const;

        float sample_1(Sampler_3D const& sampler, float3 const& uvw) const;

        float2 sample_2(Sampler_3D const& sampler, float3 const& uvw) const;

        float3 sample_3(Sampler_3D const& sampler, float3 const& uvw) const;

        float4 sample_4(Sampler_3D const& sampler, float3 const& uvw) const;

        float3 address(Sampler_3D const& sampler, float3 const& uvw) const;


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
        Invalid
    };

    Type type_;

    float scale_;

    int3 dimensions_;

    union {
        Byte1_unorm    byte1_unorm_;
        Byte2_snorm    byte2_snorm_;
        Byte2_unorm    byte2_unorm_;
        Byte3_snorm    byte3_snorm_;
        Byte3_unorm    byte3_unorm_;
        Byte3_sRGB     byte3_srgb_;
        Byte4_sRGB     byte4_srgb_;
        Half3          half3_;
        Float1         float1_;
        Float1_sparse  float1_sparse_;
        Float2         float2_;
        Float3         float3_;
    };
};

}  // namespace image::texture

#endif
