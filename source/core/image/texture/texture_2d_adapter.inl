#pragma once

#include "texture_2d_adapter.hpp"
#include "texture_2d.hpp"
#include "sampler/sampler_2d.hpp"

namespace image { namespace texture {

inline Texture_2D_adapter::Texture_2D_adapter(std::shared_ptr<Texture_2D> texture) :
	texture_(texture), scale_(float2(1.f, 1.f)) {}

inline Texture_2D_adapter::Texture_2D_adapter(std::shared_ptr<Texture_2D> texture, float2 scale) :
	texture_(texture), scale_(scale) {}

inline Texture_2D_adapter::~Texture_2D_adapter() {}

inline bool Texture_2D_adapter::is_valid() const {
	return !texture_ == false;
}

inline float Texture_2D_adapter::sample_1(const Texture_sampler_2D& sampler, float2 uv) const {
	return sampler.sample_1(*texture_, scale_ * uv);
}

inline float2 Texture_2D_adapter::sample_2(const Texture_sampler_2D& sampler, float2 uv) const {
	return sampler.sample_2(*texture_, scale_ * uv);
}

inline float3 Texture_2D_adapter::sample_3(const Texture_sampler_2D& sampler, float2 uv) const {
	return sampler.sample_3(*texture_, scale_ * uv);
}

inline float  Texture_2D_adapter::sample_1(const Texture_sampler_2D& sampler, float2 uv,
										   int32_t element) const {
	return sampler.sample_1(*texture_, scale_ * uv, element);
}

inline float2 Texture_2D_adapter::sample_2(const Texture_sampler_2D& sampler, float2 uv,
										   int32_t element) const {
	return sampler.sample_2(*texture_, scale_ * uv, element);
}

inline float3 Texture_2D_adapter::sample_3(const Texture_sampler_2D& sampler, float2 uv,
										   int32_t element) const {
	return sampler.sample_3(*texture_, scale_ * uv, element);
}

inline float2 Texture_2D_adapter::address(float2 uv) const {
	return sampler.address(scale_ * uv);
}

}}


