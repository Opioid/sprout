#pragma once

#include "sampler_3d.hpp"

namespace image { namespace texture { namespace sampler {

template<typename Address_mode>
class Nearest_3D : public Sampler_3D {

public:

	virtual float  sample_1(const Texture& texture, const float3& uvw) const override final;
	virtual float2 sample_2(const Texture& texture, const float3& uvw) const override final;
	virtual float3 sample_3(const Texture& texture, const float3& uvw) const override final;

	virtual float3 address(const float3& uvw) const override final;

private:

	static int3 map(const Texture& texture, const float3& uvw);
};

}}}
