#ifndef SU_CORE_IMAGE_TEXTURE_ADAPTER_INL
#define SU_CORE_IMAGE_TEXTURE_ADAPTER_INL

#include "base/math/vector4.inl"
#include "scene/scene.inl"
#include "scene/scene_worker.inl"
#include "texture_adapter.hpp"
#include "texture_sampler.hpp"

namespace image::texture {

inline Adapter::Adapter() : texture_(0xFFFFFFFF), scale_(1.f) {}

inline Adapter::Adapter(uint32_t texture) : texture_(texture), scale_(1.f) {}

inline Adapter::Adapter(uint32_t texture, float scale) : texture_(texture), scale_(scale) {}

inline Adapter::~Adapter() {}

inline bool Adapter::operator==(Adapter const& other) const {
    return texture_ == other.texture_ && scale_ == other.scale_;
}

inline bool Adapter::is_valid() const {
    return texture_ != 0xFFFFFFFF;
}

inline float Adapter::sample_1(Worker const& worker, Sampler_2D const& sampler, float2 uv) const {
    return sampler.sample_1(*worker.texture(texture_), scale_ * uv);
}

inline float2 Adapter::sample_2(Worker const& worker, Sampler_2D const& sampler, float2 uv) const {
    return sampler.sample_2(*worker.texture(texture_), scale_ * uv);
}

inline float3 Adapter::sample_3(Worker const& worker, Sampler_2D const& sampler, float2 uv) const {
    return sampler.sample_3(*worker.texture(texture_), scale_ * uv);
}

inline float Adapter::sample_1(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                               int32_t element) const {
    return sampler.sample_1(*worker.texture(texture_), scale_ * uv, element);
}

inline float2 Adapter::sample_2(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                                int32_t element) const {
    return sampler.sample_2(*worker.texture(texture_), scale_ * uv, element);
}

inline float3 Adapter::sample_3(Worker const& worker, Sampler_2D const& sampler, float2 uv,
                                int32_t element) const {
    return sampler.sample_3(*worker.texture(texture_), scale_ * uv, element);
}

inline float2 Adapter::address(Sampler_2D const& sampler, float2 uv) const {
    return sampler.address(scale_ * uv);
}

inline float Adapter::sample_1(Worker const& worker, Sampler_3D const& sampler,
                               float3_p uvw) const {
    return sampler.sample_1(*worker.texture(texture_), uvw);
}

inline float2 Adapter::sample_2(Worker const& worker, Sampler_3D const& sampler,
                                float3_p uvw) const {
    return sampler.sample_2(*worker.texture(texture_), uvw);
}

inline float3 Adapter::sample_3(Worker const& worker, Sampler_3D const& sampler,
                                float3_p uvw) const {
    return sampler.sample_3(*worker.texture(texture_), uvw);
}

inline float4 Adapter::sample_4(Worker const& worker, Sampler_3D const& sampler,
                                float3_p uvw) const {
    return sampler.sample_4(*worker.texture(texture_), uvw);
}

inline float3 Adapter::address(Sampler_3D const& sampler, float3_p uvw) const {
    return sampler.address(uvw);
}

}  // namespace image::texture

#endif
