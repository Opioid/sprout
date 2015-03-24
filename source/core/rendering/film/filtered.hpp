#pragma once

#include "film.hpp"

namespace rendering { namespace film {

namespace filter {

class Filter;

}

class Filtered : public Film {
public:

	Filtered(const math::uint2& dimensions, float exposure, tonemapping::Tonemapper* tonemapper, filter::Filter* filter);
	~Filtered();

	virtual void add_sample(const sampler::Camera_sample& sample, const math::float3& color, const Rectui& tile);

private:

	filter::Filter* filter_;
};

}}
