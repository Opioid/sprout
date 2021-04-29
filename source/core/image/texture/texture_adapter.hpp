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

    Adapter();
    Adapter(uint32_t texture);
    Adapter(uint32_t texture, float scale);

    ~Adapter();

    bool operator==(Adapter const& other) const;

    bool is_valid() const;

    float  sample_1(Worker const& worker, Sampler_2D const& sampler, float2 uv) const;
    float2 sample_2(Worker const& worker, Sampler_2D const& sampler, float2 uv) const;
    float3 sample_3(Worker const& worker, Sampler_2D const& sampler, float2 uv) const;

    float sample_1(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                   int32_t element) const;

    float2 sample_2(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                    int32_t element) const;

    float3 sample_3(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                    int32_t element) const;

    float2 address(Sampler_2D const& sampler, float2 uv) const;

    float sample_1(Worker const& worker, Sampler_3D const& sampler, float3_p uvw) const;

    float2 sample_2(Worker const& worker, Sampler_3D const& sampler, float3_p uvw) const;

    float3 sample_3(Worker const& worker, Sampler_3D const& sampler, float3_p uvw) const;

    float4 sample_4(Worker const& worker, Sampler_3D const& sampler, float3_p uvw) const;

    float3 address(Sampler_3D const& sampler, float3_p uvw) const;

  private:
    uint32_t texture_;

    float scale_;
};

}  // namespace image::texture

#endif
