#pragma once

#include "base/math/vector.hpp"
#include <memory>

namespace image { namespace texture {

namespace sampler { class Sampler_2d; class Sampler_3d; }

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

	using Sampler_2d = sampler::Sampler_2d;

	float  sample_1(const Sampler_2d& sampler, float2 uv) const;
	float2 sample_2(const Sampler_2d& sampler, float2 uv) const;
	float3 sample_3(const Sampler_2d& sampler, float2 uv) const;

	float  sample_1(const Sampler_2d& sampler, float2 uv, int32_t element) const;
	float2 sample_2(const Sampler_2d& sampler, float2 uv, int32_t element) const;
	float3 sample_3(const Sampler_2d& sampler, float2 uv, int32_t element) const;

	float2 address(const Sampler_2d& sampler, float2 uv) const;

	using Sampler_3d = sampler::Sampler_3d;

	float  sample_1(const Sampler_3d& sampler, float3_p uvw) const;
	float2 sample_2(const Sampler_3d& sampler, float3_p uvw) const;
	float3 sample_3(const Sampler_3d& sampler, float3_p uvw) const;

	float3 address(const Sampler_3d& sampler, float3_p uvw) const;

private:

	std::shared_ptr<Texture> texture_;

	float2 scale_;
};

}}

