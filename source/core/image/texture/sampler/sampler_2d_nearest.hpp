#pragma once

#include "sampler_2d.hpp"

namespace image { namespace sampler {

template<typename Address_mode>
class Sampler_2D_nearest : public Sampler_2D {
public:

	virtual math::float3 sample3(const Texture_2D& texture, math::float2 uv) const;

private:

	Address_mode address_mode_;
};

}}
