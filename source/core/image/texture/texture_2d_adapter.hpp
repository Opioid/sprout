#pragma once

#include "base/math/vector.hpp"
#include <memory>

namespace image { namespace texture {

namespace sampler { class Sampler_2D; }

class Texture_2D;

class Texture_2D_adapter {

public:

	Texture_2D_adapter(std::shared_ptr<Texture_2D> texture);
	Texture_2D_adapter(std::shared_ptr<Texture_2D> texture, float2 scale);
	~Texture_2D_adapter();

	bool is_valid() const;

	using Texture_sampler_2D = sampler::Sampler_2D;

	float  sample_1(const Texture_sampler_2D& sampler, float2 uv) const;
	float2 sample_2(const Texture_sampler_2D& sampler, float2 uv) const;
	float3 sample_3(const Texture_sampler_2D& sampler, float2 uv) const;

	float  sample_1(const Texture_sampler_2D& sampler, float2 uv, int32_t element) const;
	float2 sample_2(const Texture_sampler_2D& sampler, float2 uv, int32_t element) const;
	float3 sample_3(const Texture_sampler_2D& sampler, float2 uv, int32_t element) const;

	float2 address(float2 uv) const;

private:

	std::shared_ptr<Texture_2D> texture_;

	float2 scale_;
};

}}

