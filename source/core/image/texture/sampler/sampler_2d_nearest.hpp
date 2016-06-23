#pragma once

#include "sampler_2d.hpp"

namespace image { namespace texture { namespace sampler {

template<typename Address_mode>
class Sampler_2D_nearest : public Sampler_2D {
public:

	virtual float        sample_1(const Texture_2D& texture, float2 uv) const final override;
	virtual float2 sample_2(const Texture_2D& texture, float2 uv) const final override;
	virtual float3 sample_3(const Texture_2D& texture, float2 uv) const final override;

	virtual float        sample_1(const Texture_2D& texture, float2 uv, int32_t element) const final override;
	virtual float2 sample_2(const Texture_2D& texture, float2 uv, int32_t element) const final override;
	virtual float3 sample_3(const Texture_2D& texture, float2 uv, int32_t element) const final override;

	virtual float2 address(float2 uv) const final override;

private:

	Address_mode address_mode_;
};

}}}
