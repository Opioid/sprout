#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_HPP
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_HPP

#include "base/math/vector3.hpp"

namespace image::texture {

class Texture;

class Sampler_2D {
  public:
    virtual ~Sampler_2D();

    virtual float  sample_1(Texture const& texture, float2 uv) const = 0;
    virtual float2 sample_2(Texture const& texture, float2 uv) const = 0;
    virtual float3 sample_3(Texture const& texture, float2 uv) const = 0;

    virtual float  sample_1(Texture const& texture, float2 uv, int32_t element) const = 0;
    virtual float2 sample_2(Texture const& texture, float2 uv, int32_t element) const = 0;
    virtual float3 sample_3(Texture const& texture, float2 uv, int32_t element) const = 0;

    virtual float2 address(float2 uv) const = 0;
};

template <typename Address_U, typename Address_V>
class Nearest_2D final : public Sampler_2D {
  public:
    float  sample_1(Texture const& texture, float2 uv) const final;
    float2 sample_2(Texture const& texture, float2 uv) const final;
    float3 sample_3(Texture const& texture, float2 uv) const final;

    float sample_1(Texture const& texture, float2 uv, int32_t element) const final;
    float2 sample_2(Texture const& texture, float2 uv, int32_t element) const final;
    float3 sample_3(Texture const& texture, float2 uv, int32_t element) const final;

    float2 address(float2 uv) const final;

  private:
    static int2 map(Texture const& texture, float2 uv);
};

template <typename Address_U, typename Address_V>
class Linear_2D : public Sampler_2D {
  public:
    float  sample_1(Texture const& texture, float2 uv) const final;
    float2 sample_2(Texture const& texture, float2 uv) const final;
    float3 sample_3(Texture const& texture, float2 uv) const final;

    float sample_1(Texture const& texture, float2 uv, int32_t element) const final;
    float2 sample_2(Texture const& texture, float2 uv, int32_t element) const final;
    float3 sample_3(Texture const& texture, float2 uv, int32_t element) const final;

    float2 address(float2 uv) const final;

  private:
    static float2 map(Texture const& texture, float2 uv, int4& xy_xy1);
};

class Sampler_3D {
  public:
    virtual ~Sampler_3D();

    virtual float  sample_1(Texture const& texture, float3_p uvw) const = 0;
    virtual float2 sample_2(Texture const& texture, float3_p uvw) const = 0;
    virtual float3 sample_3(Texture const& texture, float3_p uvw) const = 0;
    virtual float4 sample_4(Texture const& texture, float3_p uvw) const = 0;

    virtual float  stochastic_1(Texture const& texture, float3_p uvw, float3_p r) const = 0;
    virtual float2 stochastic_2(Texture const& texture, float3_p uvw, float3_p r) const = 0;
    virtual float3 stochastic_3(Texture const& texture, float3_p uvw, float3_p r) const = 0;
    virtual float4 stochastic_4(Texture const& texture, float3_p uvw, float3_p r) const = 0;

    virtual float3 address(float3_p uvw) const = 0;
};

template <typename Address_mode>
class Stochastic_3D : public Sampler_3D {
  public:

    virtual ~Stochastic_3D();

    float  stochastic_1(Texture const& texture, float3_p uvw, float3_p r) const final;
    float2 stochastic_2(Texture const& texture, float3_p uvw, float3_p r) const final;
    float3 stochastic_3(Texture const& texture, float3_p uvw, float3_p r) const final;
    float4 stochastic_4(Texture const& texture, float3_p uvw, float3_p r) const final;

    float3 address(float3_p uvw) const final;

  private:
    static int3 map(Texture const& texture, float3_p uvw, float3_p r);
};

template <typename Address_mode>
class Nearest_3D : public Stochastic_3D<Address_mode> {
  public:
    float  sample_1(Texture const& texture, float3_p uvw) const final;
    float2 sample_2(Texture const& texture, float3_p uvw) const final;
    float3 sample_3(Texture const& texture, float3_p uvw) const final;
    float4 sample_4(Texture const& texture, float3_p uvw) const final;

private:
  static int3 map(Texture const& texture, float3_p uvw);
};

template <typename Address_mode>
class Linear_3D : public Stochastic_3D<Address_mode> {
  public:
    float  sample_1(Texture const& texture, float3_p uvw) const final;
    float2 sample_2(Texture const& texture, float3_p uvw) const final;
    float3 sample_3(Texture const& texture, float3_p uvw) const final;
    float4 sample_4(Texture const& texture, float3_p uvw) const final;

  private:
    static float3 map(Texture const& texture, float3_p uvw, int3& xyz, int3& xyz1);
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

extern template class Stochastic_3D<Address_mode_clamp>;
extern template class Stochastic_3D<Address_mode_repeat>;

extern template class Nearest_3D<Address_mode_clamp>;
extern template class Nearest_3D<Address_mode_repeat>;

extern template class Linear_3D<Address_mode_clamp>;
extern template class Linear_3D<Address_mode_repeat>;

}  // namespace image::texture

#endif
