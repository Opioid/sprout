#pragma once

#include "sampler_3d.hpp"

namespace image { namespace texture { namespace sampler {

template<typename Address_mode>
class Linear_3D : public Sampler_3D {

public:

	virtual float  sample_1(const Texture& texture, float3_p uvw) const final override;
	virtual float2 sample_2(const Texture& texture, float3_p uvw) const final override;
	virtual float3 sample_3(const Texture& texture, float3_p uvw) const final override;

	virtual float3 address(float3_p uvw) const final override;

private:

	static float3 map(const Texture& texture, float3_p uvw, int3& xyz, int3& xyz1);
};

}}}
