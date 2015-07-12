#pragma once

#include "sampler_spherical.hpp"

namespace image { namespace texture { namespace sampler {

class Spherical_nearest : public Spherical {
public:

	virtual math::float3 sample_3(const Texture_2D& texture, const math::float3& xyz) const;
};

}}}
