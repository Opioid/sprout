#pragma once

#include "texture_adapter.hpp"
#include "texture.hpp"
#include "sampler/sampler_2d.hpp"
#include "sampler/sampler_3d.hpp"
#include "base/math/vector3.inl"

namespace image { namespace texture {

inline Adapter::Adapter() {}

inline Adapter::Adapter(std::shared_ptr<Texture> texture) :
	texture_(texture), scale_(float2(1.f, 1.f)) {}

inline Adapter::Adapter(std::shared_ptr<Texture> texture, float2 scale) :
	texture_(texture), scale_(scale) {}

inline Adapter::~Adapter() {}

inline bool Adapter::operator==(const Adapter& other) const {
	return texture_ == other.texture_ && scale_ == other.scale_;
}

inline bool Adapter::is_valid() const {
	return !texture_ == false;
}

inline const Texture* Adapter::texture() const {
	return texture_.get();
}

inline float Adapter::sample_1(const Sampler_2D& sampler, float2 uv) const {
	return sampler.sample_1(*texture_, scale_ * uv);
}

inline float2 Adapter::sample_2(const Sampler_2D& sampler, float2 uv) const {
	return sampler.sample_2(*texture_, scale_ * uv);
}

inline float3 Adapter::sample_3(const Sampler_2D& sampler, float2 uv) const {
	return sampler.sample_3(*texture_, scale_ * uv);
}

inline float Adapter::sample_1(const Sampler_2D& sampler, float2 uv, int32_t element) const {
	return sampler.sample_1(*texture_, scale_ * uv, element);
}

inline float2 Adapter::sample_2(const Sampler_2D& sampler, float2 uv, int32_t element) const {
	return sampler.sample_2(*texture_, scale_ * uv, element);
}

inline float3 Adapter::sample_3(const Sampler_2D& sampler, float2 uv, int32_t element) const {
	return sampler.sample_3(*texture_, scale_ * uv, element);
}

inline float2 Adapter::address(const Sampler_2D& sampler, float2 uv) const {
	return sampler.address(scale_ * uv);
}

inline float Adapter::sample_1(const Sampler_3D& sampler, const float3& uvw) const {
	return sampler.sample_1(*texture_, uvw);
}

inline float2 Adapter::sample_2(const Sampler_3D& sampler, const float3& uvw) const {
	return sampler.sample_2(*texture_, uvw);
}

inline float3 Adapter::sample_3(const Sampler_3D& sampler, const float3& uvw) const {
	return sampler.sample_3(*texture_, uvw);
}

inline float3 Adapter::address(const Sampler_3D& sampler, const float3& uvw) const {
	return sampler.address(uvw);
}

}}


