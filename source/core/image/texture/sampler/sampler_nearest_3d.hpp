#pragma once

#include "sampler_3d.hpp"

namespace image::texture::sampler {

template<typename Address_mode>
class Nearest_3D : public Sampler_3D {

public:

	virtual float  sample_1(const Texture& texture, float3 const& uvw) const override final;
	virtual float2 sample_2(const Texture& texture, float3 const& uvw) const override final;
	virtual float3 sample_3(const Texture& texture, float3 const& uvw) const override final;

	virtual float3 address(float3 const& uvw) const override final;

private:

	static int3 map(const Texture& texture, float3 const& uvw);
};

}
