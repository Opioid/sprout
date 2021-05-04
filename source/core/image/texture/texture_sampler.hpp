#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_HPP
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_HPP

#include "base/math/vector.hpp"

namespace scene {
class Scene;
}

namespace image::texture {

class Texture;
class Texture;

class Sampler_2D {
  public:
    using Scene = scene::Scene;

    virtual ~Sampler_2D();

    virtual float  sample_1(Texture const& texture, float2 uv, Scene const& scene) const = 0;
    virtual float2 sample_2(Texture const& texture, float2 uv, Scene const& scene) const = 0;
    virtual float3 sample_3(Texture const& texture, float2 uv, Scene const& scene) const = 0;

    virtual float2 address(float2 uv) const = 0;
};

template <typename Address_U, typename Address_V>
class Nearest_2D final : public Sampler_2D {
  public:
    float  sample_1(Texture const& texture, float2 uv, Scene const& scene) const final;
    float2 sample_2(Texture const& texture, float2 uv, Scene const& scene) const final;
    float3 sample_3(Texture const& texture, float2 uv, Scene const& scene) const final;

    float2 address(float2 uv) const final;

  private:
    static int2 map(int2 d, float2 uv);
};

template <typename Address_U, typename Address_V>
class Linear_2D : public Sampler_2D {
  public:
    float  sample_1(Texture const& texture, float2 uv, Scene const& scene) const final;
    float2 sample_2(Texture const& texture, float2 uv, Scene const& scene) const final;
    float3 sample_3(Texture const& texture, float2 uv, Scene const& scene) const final;

    float2 address(float2 uv) const final;

  private:
    static float2 map(int2 d, float2 uv, int4& xy_xy1);
};

class Sampler_3D {
  public:
    using Scene = scene::Scene;

    virtual ~Sampler_3D();

    virtual float  sample_1(Texture const& texture, float3_p uvw, Scene const& scene) const = 0;
    virtual float2 sample_2(Texture const& texture, float3_p uvw, Scene const& scene) const = 0;
    virtual float3 sample_3(Texture const& texture, float3_p uvw, Scene const& scene) const = 0;
    virtual float4 sample_4(Texture const& texture, float3_p uvw, Scene const& scene) const = 0;

    virtual float3 address(float3_p uvw) const = 0;
};

template <typename Address_mode>
class Nearest_3D : public Sampler_3D {
  public:
    float  sample_1(Texture const& texture, float3_p uvw, Scene const& scene) const final;
    float2 sample_2(Texture const& texture, float3_p uvw, Scene const& scene) const final;
    float3 sample_3(Texture const& texture, float3_p uvw, Scene const& scene) const final;
    float4 sample_4(Texture const& texture, float3_p uvw, Scene const& scene) const final;

    float3 address(float3_p uvw) const final;

  private:
    static int3 map(int3 d, float3_p uvw);
};

template <typename Address_mode>
class Linear_3D : public Sampler_3D {
  public:
    float  sample_1(Texture const& texture, float3_p uvw, Scene const& scene) const final;
    float2 sample_2(Texture const& texture, float3_p uvw, Scene const& scene) const final;
    float3 sample_3(Texture const& texture, float3_p uvw, Scene const& scene) const final;
    float4 sample_4(Texture const& texture, float3_p uvw, Scene const& scene) const final;

    float3 address(float3_p uvw) const final;

  private:
    static float3 map(int3 d, float3_p uvw, int3& xyz, int3& xyz1);
};

struct Address_mode_clamp;
struct Address_mode_repeat;

extern template class Nearest_2D<Address_mode_clamp, Address_mode_clamp>;
extern template class Nearest_2D<Address_mode_clamp, Address_mode_repeat>;
extern template class Nearest_2D<Address_mode_repeat, Address_mode_clamp>;
extern template class Nearest_2D<Address_mode_repeat, Address_mode_repeat>;

extern template class Linear_2D<Address_mode_clamp, Address_mode_clamp>;
extern template class Linear_2D<Address_mode_clamp, Address_mode_repeat>;
extern template class Linear_2D<Address_mode_repeat, Address_mode_clamp>;
extern template class Linear_2D<Address_mode_repeat, Address_mode_repeat>;

extern template class Nearest_3D<Address_mode_clamp>;
extern template class Nearest_3D<Address_mode_repeat>;

extern template class Linear_3D<Address_mode_clamp>;
extern template class Linear_3D<Address_mode_repeat>;

}  // namespace image::texture

#endif
