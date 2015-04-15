#pragma once

#include "sampler_spherical.hpp"

namespace image { namespace sampler {

class Spherical_nearest : public Spherical {
public:

	virtual math::float3 sample3(const Texture2D& texture, const math::float3& xyz) const;
};

}}
