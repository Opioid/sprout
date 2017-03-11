#pragma once

#include "sampler_2d.hpp"
#include "base/math/vector4.hpp"

namespace image { namespace texture { namespace sampler {

template<typename Address_U, typename Address_V>
class Linear_2D : public Sampler_2D {

public:

	virtual float  sample_1(const Texture& texture, float2 uv) const final override;

	virtual float2 sample_2(const Texture& texture, float2 uv) const final override;

	template<uint32_t Axis>
	float2 sample_2(const Texture& texture, float2 uv, float weight, int2 x_x1) const;

	virtual float3 sample_3(const Texture& texture, float2 uv) const final override;

	virtual float  sample_1(const Texture& texture, float2 uv,
							int32_t element) const final override;

	virtual float2 sample_2(const Texture& texture, float2 uv,
							int32_t element) const final override;

	virtual float3 sample_3(const Texture& texture, float2 uv,
							int32_t element) const final override;

	virtual float2 address(float2 uv) const final override;

	template<uint32_t Axis>
	static float map(const Texture& texture, float tc, int2& x_x1);

private:

	static float2 map(const Texture& texture, float2 uv, int4& xy_xy1);
};

}}}

