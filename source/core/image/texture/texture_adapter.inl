#ifndef SU_CORE_IMAGE_TEXTURE_ADAPTER_INL
#define SU_CORE_IMAGE_TEXTURE_ADAPTER_INL

#include "base/math/vector3.inl"
#include "sampler/sampler_2d.hpp"
#include "sampler/sampler_3d.hpp"
#include "texture.hpp"
#include "texture_adapter.hpp"

namespace image::texture {

inline Adapter::Adapter() {}

inline Adapter::Adapter(std::shared_ptr<Texture> const& texture)
    : texture_(texture), scale_(float2(1.f, 1.f)) {}

inline Adapter::Adapter(std::shared_ptr<Texture> const& texture, float2 scale)
    : texture_(texture), scale_(scale) {}

inline Adapter::~Adapter() {}

inline bool Adapter::operator==(const Adapter& other) const {
    return texture_ == other.texture_ && scale_ == other.scale_;
}

inline bool Adapter::is_valid() const {
    return !texture_ == false;
}

inline Texture const* Adapter::texture() const {
    return texture_.get();
}

inline float Adapter::sample_1(Sampler_2D const& sampler, float2 uv) const noexcept {
    return sampler.sample_1(*texture_, scale_ * uv);
}

inline float2 Adapter::sample_2(Sampler_2D const& sampler, float2 uv) const noexcept {
    return sampler.sample_2(*texture_, scale_ * uv);
}

inline float3 Adapter::sample_3(Sampler_2D const& sampler, float2 uv) const noexcept {
    return sampler.sample_3(*texture_, scale_ * uv);
}

inline float Adapter::sample_1(Sampler_2D const& sampler, float2 uv, int32_t element) const
    noexcept {
    return sampler.sample_1(*texture_, scale_ * uv, element);
}

inline float2 Adapter::sample_2(Sampler_2D const& sampler, float2 uv, int32_t element) const
    noexcept {
    return sampler.sample_2(*texture_, scale_ * uv, element);
}

inline float3 Adapter::sample_3(Sampler_2D const& sampler, float2 uv, int32_t element) const
    noexcept {
    return sampler.sample_3(*texture_, scale_ * uv, element);
}

inline float2 Adapter::address(Sampler_2D const& sampler, float2 uv) const noexcept {
    return sampler.address(scale_ * uv);
}

inline float Adapter::sample_1(Sampler_3D const& sampler, f_float3 uvw) const noexcept {
    return sampler.sample_1(*texture_, uvw);
}

inline float2 Adapter::sample_2(Sampler_3D const& sampler, f_float3 uvw) const noexcept {
    return sampler.sample_2(*texture_, uvw);
}

inline float3 Adapter::sample_3(Sampler_3D const& sampler, f_float3 uvw) const noexcept {
    return sampler.sample_3(*texture_, uvw);
}

inline float3 Adapter::address(Sampler_3D const& sampler, f_float3 uvw) const noexcept {
    return sampler.address(uvw);
}

}  // namespace image::texture

#endif
