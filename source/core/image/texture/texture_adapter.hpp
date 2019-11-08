#ifndef SU_CORE_IMAGE_TEXTURE_ADAPTER_HPP
#define SU_CORE_IMAGE_TEXTURE_ADAPTER_HPP

#include "base/math/vector2.hpp"

namespace scene {
class Worker;
class Scene;
}  // namespace scene

namespace image::texture {

class Sampler_2D;
class Sampler_3D;

class Texture;

class Adapter {
  public:
    using Scene  = scene::Scene;
    using Worker = scene::Worker;

    Adapter() noexcept;
    Adapter(uint32_t texture) noexcept;
    Adapter(uint32_t texture, float scale) noexcept;

    ~Adapter();

    bool operator==(Adapter const& other) const noexcept;

    bool is_valid() const noexcept;

    Texture const& texture(Scene const& scene) const noexcept;

    float  sample_1(Worker const& worker, Sampler_2D const& sampler, float2 uv) const noexcept;
    float2 sample_2(Worker const& worker, Sampler_2D const& sampler, float2 uv) const noexcept;
    float3 sample_3(Worker const& worker, Sampler_2D const& sampler, float2 uv) const noexcept;

    float sample_1(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                   int32_t element) const noexcept;

    float2 sample_2(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                    int32_t element) const noexcept;

    float3 sample_3(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                    int32_t element) const noexcept;

    float2 address(Sampler_2D const& sampler, float2 uv) const noexcept;

    float sample_1(Worker const& worker, Sampler_3D const& sampler, float3 const& uvw) const
        noexcept;

    float2 sample_2(Worker const& worker, Sampler_3D const& sampler, float3 const& uvw) const
        noexcept;

    float3 sample_3(Worker const& worker, Sampler_3D const& sampler, float3 const& uvw) const
        noexcept;

    float4 sample_4(Worker const& worker, Sampler_3D const& sampler, float3 const& uvw) const
        noexcept;

    float3 address(Sampler_3D const& sampler, float3 const& uvw) const noexcept;

  private:
    uint32_t texture_;

    float scale_;
};

}  // namespace image::texture

#endif
