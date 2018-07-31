#ifndef SU_CORE_IMAGE_TEXTURE_ADAPTER_HPP
#define SU_CORE_IMAGE_TEXTURE_ADAPTER_HPP

#include <memory>
#include "base/math/vector2.hpp"

namespace image::texture {

namespace sampler {
class Sampler_2D;
class Sampler_3D;
}  // namespace sampler

class Texture;

class Adapter {
  public:
    Adapter();
    Adapter(std::shared_ptr<Texture> const& texture);
    Adapter(std::shared_ptr<Texture> const& texture, float2 scale);
    ~Adapter();

    bool operator==(const Adapter& other) const;

    bool is_valid() const;

    Texture const* texture() const;

    using Sampler_2D = sampler::Sampler_2D;

    float  sample_1(Sampler_2D const& sampler, float2 uv) const noexcept;
    float2 sample_2(Sampler_2D const& sampler, float2 uv) const noexcept;
    float3 sample_3(Sampler_2D const& sampler, float2 uv) const noexcept;

    float  sample_1(Sampler_2D const& sampler, float2 uv, int32_t element) const noexcept;
    float2 sample_2(Sampler_2D const& sampler, float2 uv, int32_t element) const noexcept;
    float3 sample_3(Sampler_2D const& sampler, float2 uv, int32_t element) const noexcept;

    float2 address(Sampler_2D const& sampler, float2 uv) const noexcept;

    using Sampler_3D = sampler::Sampler_3D;

    float  sample_1(Sampler_3D const& sampler, f_float3 uvw) const noexcept;
    float2 sample_2(Sampler_3D const& sampler, f_float3 uvw) const noexcept;
    float3 sample_3(Sampler_3D const& sampler, f_float3 uvw) const noexcept;

    float3 address(Sampler_3D const& sampler, f_float3 uvw) const noexcept;

  private:
    std::shared_ptr<Texture> texture_;

    float2 scale_;
};

}  // namespace image::texture

#endif
