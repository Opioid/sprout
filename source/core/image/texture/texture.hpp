#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP

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

#include "base/math/vector2.hpp"

namespace scene {
class Scene;
}  // namespace scene

namespace image::texture {

class alignas(16) Texture {
  public:
    static char const* identifier();

    uint32_t image_id_;

    Texture(Byte1_unorm const& texture, uint32_t image_id);
    Texture(Byte2_snorm const& texture, uint32_t image_id);
    Texture(Byte2_unorm const& texture, uint32_t image_id);
    Texture(Byte3_snorm const& texture, uint32_t image_id);
    Texture(Byte3_unorm const& texture, uint32_t image_id);
    Texture(Byte3_sRGB const& texture, uint32_t image_id);
    Texture(Byte4_sRGB const& texture, uint32_t image_id);
    Texture(Half3 const& texture, uint32_t image_id);
    Texture(Float1 const& texture, uint32_t image_id);
    Texture(Float1_sparse const& texture, uint32_t image_id);
    Texture(Float2 const& texture, uint32_t image_id);
    Texture(Float3 const& texture, uint32_t image_id);

    int32_t num_channels() const;

    int32_t num_elements() const;

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

    void gather_1(int3_p xyz, int3_p xyz1, float c[8]) const;
    void gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const;

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

class Sampler_2D;

class Turbotexture {
public:

    using Scene = scene::Scene;

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

    Turbotexture();

    Turbotexture(Type type, uint32_t image);

    Description const& description(Scene const& scene) const;

    float2 scale() const;

    float3 at_3(int32_t x, int32_t y, Scene const& scene) const;

    void gather_3(int4_p xy_xy1, Scene const& scene, float3 c[4]) const;

private:


    Type type_;

    uint32_t image_id_;

    float2 scale_;
};

}  // namespace image::texture

#endif
