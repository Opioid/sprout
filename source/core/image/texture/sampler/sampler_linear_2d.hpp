#pragma once

#include "sampler_2d.hpp"
#include "base/math/vector4.hpp"

namespace image::texture::sampler {

template<typename Address_U, typename Address_V>
class Linear_2D : public Sampler_2D {

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

	static float2 map(const Texture& texture, float2 uv, int4& xy_xy1);
};

}
