#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_3D
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_3D

#include "base/math/vector.hpp"

namespace image::texture {

class Texture;

namespace sampler {

class Sampler_3D {

public:

	virtual ~Sampler_3D() {}

	virtual float  sample_1(const Texture& texture, float3 const& uvw) const = 0;
	virtual float2 sample_2(const Texture& texture, float3 const& uvw) const = 0;
	virtual float3 sample_3(const Texture& texture, float3 const& uvw) const = 0;

	virtual float3 address(float3 const& uvw) const = 0;
};

}}

#endif
