#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP

#include "image/typed_image_fwd.hpp"
#include "base/math/vector2.hpp"

namespace scene {
class Scene;
}  // namespace scene

namespace image::texture {

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

    bool operator==(Turbotexture const& other) const;

    bool is_valid() const;

    int32_t num_channels() const;

    Description const& description(Scene const& scene) const;

    float2 scale() const;

    float  at_1(int32_t x, int32_t y, Scene const& scene) const;
    float2 at_2(int32_t x, int32_t y, Scene const& scene) const;
    float3 at_3(int32_t x, int32_t y, Scene const& scene) const;
    float4 at_4(int32_t x, int32_t y, Scene const& scene) const;

    void gather_1(int4_p xy_xy1, Scene const& scene, float  c[4]) const;
    void gather_2(int4_p xy_xy1, Scene const& scene, float2 c[4]) const;
    void gather_3(int4_p xy_xy1, Scene const& scene, float3 c[4]) const;

    float  at_1(int32_t x, int32_t y, int32_t z, Scene const& scene) const;
    float2 at_2(int32_t x, int32_t y, int32_t z, Scene const& scene) const;
    float3 at_3(int32_t x, int32_t y, int32_t z, Scene const& scene) const;
    float4 at_4(int32_t x, int32_t y, int32_t z, Scene const& scene) const;

    void gather_1(int3_p xyz, int3_p xyz1, Scene const& scene, float c[8]) const;
    void gather_2(int3_p xyz, int3_p xyz1, Scene const& scene, float2 c[8]) const;

    float average_1(Scene const& scene) const;
    float3 average_3(Scene const& scene) const;

private:

    Type type_;

    uint32_t image_id_;

    float2 scale_;
};

}  // namespace image::texture

#endif
