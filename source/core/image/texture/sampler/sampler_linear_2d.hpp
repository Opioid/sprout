#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_2D_HPP
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_2D_HPP

#include "sampler_2d.hpp"
#include "base/math/vector4.hpp"

namespace image::texture::sampler {

template<typename Address_U, typename Address_V>
class Linear_2D : public Sampler_2D {

public:

	virtual float  sample_1(Texture const& texture, float2 uv) const override final;

	virtual float2 sample_2(Texture const& texture, float2 uv) const override final;

	virtual float3 sample_3(Texture const& texture, float2 uv) const override final;

	virtual float  sample_1(Texture const& texture, float2 uv,
							int32_t element) const override final;

	virtual float2 sample_2(Texture const& texture, float2 uv,
							int32_t element) const override final;

	virtual float3 sample_3(Texture const& texture, float2 uv,
							int32_t element) const override final;

	virtual float2 address(float2 uv) const override final;

private:

	static float2 map(Texture const& texture, float2 uv, int4& xy_xy1);
};

}

#endif
