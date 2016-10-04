#pragma once

#include "base/math/vector.hpp"
#include <memory>

namespace image { namespace texture {

namespace sampler { class Sampler_2D; }

class Texture;

class Adapter {

public:

	Adapter();
	Adapter(std::shared_ptr<Texture> texture);
	Adapter(std::shared_ptr<Texture> texture, float2 scale);
	~Adapter();

	bool operator==(const Adapter& other) const;

	bool is_valid() const;

	const Texture* texture() const;

	using Sampler_2D = sampler::Sampler_2D;

	float  sample_1(const Sampler_2D& sampler, float2 uv) const;
	float2 sample_2(const Sampler_2D& sampler, float2 uv) const;
	float3 sample_3(const Sampler_2D& sampler, float2 uv) const;

	float  sample_1(const Sampler_2D& sampler, float2 uv, int32_t element) const;
	float2 sample_2(const Sampler_2D& sampler, float2 uv, int32_t element) const;
	float3 sample_3(const Sampler_2D& sampler, float2 uv, int32_t element) const;

	float2 address(const Sampler_2D& sampler, float2 uv) const;

private:

	std::shared_ptr<Texture> texture_;

	float2 scale_;
};

}}

