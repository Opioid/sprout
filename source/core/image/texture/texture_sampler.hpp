#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_HPP
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_HPP

#include "base/math/vector.hpp"

namespace image::texture {

class Texture;

class Sampler_2D {
  public:
    virtual ~Sampler_2D() noexcept;

    virtual float  sample_1(Texture const& texture, float2 uv) const noexcept = 0;
    virtual float2 sample_2(Texture const& texture, float2 uv) const noexcept = 0;
    virtual float3 sample_3(Texture const& texture, float2 uv) const noexcept = 0;

    virtual float  sample_1(Texture const& texture, float2 uv, int32_t element) const noexcept = 0;
    virtual float2 sample_2(Texture const& texture, float2 uv, int32_t element) const noexcept = 0;
    virtual float3 sample_3(Texture const& texture, float2 uv, int32_t element) const noexcept = 0;

    virtual float2 address(float2 uv) const noexcept = 0;
};

template <typename Address_mode_U, typename Address_mode_V>
class Nearest_2D final : public Sampler_2D {
  public:
    float  sample_1(Texture const& texture, float2 uv) const noexcept final;
    float2 sample_2(Texture const& texture, float2 uv) const noexcept final;
    float3 sample_3(Texture const& texture, float2 uv) const noexcept final;

    float sample_1(Texture const& texture, float2 uv, int32_t element) const noexcept final;

    float2 sample_2(Texture const& texture, float2 uv, int32_t element) const noexcept final;

    float3 sample_3(Texture const& texture, float2 uv, int32_t element) const noexcept final;

    float2 address(float2 uv) const noexcept final;

  private:
    static int2 map(Texture const& texture, float2 uv) noexcept;
};

template <typename Address_U, typename Address_V>
class Linear_2D : public Sampler_2D {
  public:
    float  sample_1(Texture const& texture, float2 uv) const noexcept final;
    float2 sample_2(Texture const& texture, float2 uv) const noexcept final;
    float3 sample_3(Texture const& texture, float2 uv) const noexcept final;

    float sample_1(Texture const& texture, float2 uv, int32_t element) const noexcept final;

    float2 sample_2(Texture const& texture, float2 uv, int32_t element) const noexcept final;

    float3 sample_3(Texture const& texture, float2 uv, int32_t element) const noexcept final;

    float2 address(float2 uv) const noexcept final;

  private:
    static float2 map(Texture const& texture, float2 uv, int4& xy_xy1) noexcept;
};

class Sampler_3D {
  public:
    virtual ~Sampler_3D() noexcept;

    virtual float  sample_1(Texture const& texture, float3 const& uvw) const noexcept = 0;
    virtual float2 sample_2(Texture const& texture, float3 const& uvw) const noexcept = 0;
    virtual float3 sample_3(Texture const& texture, float3 const& uvw) const noexcept = 0;
    virtual float4 sample_4(Texture const& texture, float3 const& uvw) const noexcept = 0;

    virtual float3 address(float3 const& uvw) const noexcept = 0;
};

template <typename Address_mode>
class Nearest_3D : public Sampler_3D {
  public:
    float  sample_1(Texture const& texture, float3 const& uvw) const noexcept final;
    float2 sample_2(Texture const& texture, float3 const& uvw) const noexcept final;
    float3 sample_3(Texture const& texture, float3 const& uvw) const noexcept final;
    float4 sample_4(Texture const& texture, float3 const& uvw) const noexcept final;

    float3 address(float3 const& uvw) const noexcept final;

  private:
    static int3 map(Texture const& texture, float3 const& uvw) noexcept;
};

template <typename Address_mode>
class Linear_3D : public Sampler_3D {
  public:
    float  sample_1(Texture const& texture, float3 const& uvw) const noexcept final;
    float2 sample_2(Texture const& texture, float3 const& uvw) const noexcept final;
    float3 sample_3(Texture const& texture, float3 const& uvw) const noexcept final;
    float4 sample_4(Texture const& texture, float3 const& uvw) const noexcept final;

    float3 address(float3 const& uvw) const noexcept final;

  private:
    static float3 map(Texture const& texture, float3 const& uvw, int3& xyz, int3& xyz1) noexcept;
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
