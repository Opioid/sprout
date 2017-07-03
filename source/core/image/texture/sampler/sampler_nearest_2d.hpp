#pragma once

#include "sampler_2d.hpp"

namespace image { namespace texture { namespace sampler {

template<typename Address_mode_U, typename Address_mode_V>
class Nearest_2D : public Sampler_2D {

public:

	virtual float  sample_1(const Texture& texture, float2 uv) const override final;
	virtual float2 sample_2(const Texture& texture, float2 uv) const override final;
	virtual float3 sample_3(const Texture& texture, float2 uv) const override final;

	virtual float  sample_1(const Texture& texture, float2 uv,
							int32_t element) const override final;

	virtual float2 sample_2(const Texture& texture, float2 uv,
							int32_t element) const override final;

	virtual float3 sample_3(const Texture& texture, float2 uv,
							int32_t element) const override final;

	virtual float2 address(float2 uv) const override final;

private:

	static int2 map(const Texture& texture, float2 uv);
};

}}}
