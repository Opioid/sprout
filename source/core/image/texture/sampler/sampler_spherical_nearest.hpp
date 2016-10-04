#pragma once

#include "sampler_spherical.hpp"

namespace image { namespace texture { namespace sampler {

class Spherical_nearest : public Spherical {
public:

	virtual float3 sample_3(const Texture& texture, const float3& xyz) const;
};

}}}
