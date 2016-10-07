#pragma once

#include "sampler_settings.hpp"
#include <cstdint>

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene { namespace material {

class Sampler_cache {

public:

	Sampler_cache();
	~Sampler_cache();

	using Sampler_2D = image::texture::sampler::Sampler_2D;

	const Sampler_2D& sampler(uint32_t key, Sampler_settings::Filter filter) const;

private:

	Sampler_2D* samplers_[2];
};

}}
