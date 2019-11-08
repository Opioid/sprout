#ifndef SU_CORE_IMAGE_TEXTURE_ADAPTER_INL
#define SU_CORE_IMAGE_TEXTURE_ADAPTER_INL

#include "base/math/vector4.inl"
#include "scene/scene.inl"
#include "scene/scene_worker.inl"
#include "texture_adapter.hpp"
#include "texture_sampler.hpp"

namespace image::texture {

inline Adapter::Adapter() noexcept : texture_(0xFFFFFFFF) {}

inline Adapter::Adapter(uint32_t texture) noexcept : texture_(texture), scale_(1.f) {}

inline Adapter::Adapter(uint32_t texture, float scale) noexcept
    : texture_(texture), scale_(scale) {}

inline Adapter::~Adapter() {}

inline bool Adapter::operator==(Adapter const& other) const noexcept {
    return texture_ == other.texture_ && scale_ == other.scale_;
}

inline bool Adapter::is_valid() const noexcept {
    return texture_ != 0xFFFFFFFF;
}

inline Texture const& Adapter::texture(Scene const& scene) const noexcept {
    return *scene.texture(texture_);
}

inline float Adapter::sample_1(Worker const& worker, Sampler_2D const& sampler, float2 uv) const
    noexcept {
    return sampler.sample_1(*worker.texture(texture_), scale_ * uv);
}

inline float2 Adapter::sample_2(Worker const& worker, Sampler_2D const& sampler, float2 uv) const
    noexcept {
    return sampler.sample_2(*worker.texture(texture_), scale_ * uv);
}

inline float3 Adapter::sample_3(Worker const& worker, Sampler_2D const& sampler, float2 uv) const
    noexcept {
    return sampler.sample_3(*worker.texture(texture_), scale_ * uv);
}

inline float Adapter::sample_1(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                               int32_t element) const noexcept {
    return sampler.sample_1(*worker.texture(texture_), scale_ * uv, element);
}

inline float2 Adapter::sample_2(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                                int32_t element) const noexcept {
    return sampler.sample_2(*worker.texture(texture_), scale_ * uv, element);
}

inline float3 Adapter::sample_3(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                                int32_t element) const noexcept {
    return sampler.sample_3(*worker.texture(texture_), scale_ * uv, element);
}

inline float2 Adapter::address(Sampler_2D const& sampler, float2 uv) const noexcept {
    return sampler.address(scale_ * uv);
}

inline float Adapter::sample_1(Worker const& worker, Sampler_3D const& sampler,
                               float3 const& uvw) const noexcept {
    return sampler.sample_1(*worker.texture(texture_), uvw);
}

inline float2 Adapter::sample_2(Worker const& worker, Sampler_3D const& sampler,
                                float3 const& uvw) const noexcept {
    return sampler.sample_2(*worker.texture(texture_), uvw);
}

inline float3 Adapter::sample_3(Worker const& worker, Sampler_3D const& sampler,
                                float3 const& uvw) const noexcept {
    return sampler.sample_3(*worker.texture(texture_), uvw);
}

inline float4 Adapter::sample_4(Worker const& worker, Sampler_3D const& sampler,
                                float3 const& uvw) const noexcept {
    return sampler.sample_4(*worker.texture(texture_), uvw);
}

inline float3 Adapter::address(Sampler_3D const& sampler, float3 const& uvw) const noexcept {
    return sampler.address(uvw);
}

}  // namespace image::texture

#endif
