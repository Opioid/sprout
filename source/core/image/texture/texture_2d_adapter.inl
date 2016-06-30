#pragma once

#include "texture_2d_adapter.hpp"
#include "texture_2d.hpp"
#include "sampler/sampler_2d.hpp"

namespace image { namespace texture {


Texture_2D_adapter::Texture_2D_adapter(std::shared_ptr<Texture_2D> texture, float2 scale) :
	texture_(texture), scale_(scale) {}

Texture_2D_adapter::~Texture_2D_adapter() {}

bool Texture_2D_adapter::is_valid() const {
	return !texture_ == false;
}

float Texture_2D_adapter::sample_1(const Texture_sampler_2D& sampler, float2 uv) const {
	return sampler.sample_1(*texture_, scale_ * uv);
}

float2 Texture_2D_adapter::sample_2(const Texture_sampler_2D& sampler, float2 uv) const {
	return sampler.sample_2(*texture_, scale_ * uv);
}

float3 Texture_2D_adapter::sample_3(const Texture_sampler_2D& sampler, float2 uv) const {
	return sampler.sample_3(*texture_, scale_ * uv);
}

float  Texture_2D_adapter::sample_1(const Texture_sampler_2D& sampler, float2 uv,
									int32_t element) const {
	return sampler.sample_1(*texture_, scale_ * uv, element);
}

float2 Texture_2D_adapter::sample_2(const Texture_sampler_2D& sampler, float2 uv,
									int32_t element) const {
	return sampler.sample_2(*texture_, scale_ * uv, element);
}

float3 Texture_2D_adapter::sample_3(const Texture_sampler_2D& sampler, float2 uv,
									int32_t element) const {
	return sampler.sample_3(*texture_, scale_ * uv, element);
}

float2 Texture_2D_adapter::address(float2 uv) const {
	return sampler.address(scale_ * uv);
}

}}


