#pragma once

#include "sampler_settings.hpp"
#include <cstdint>

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene { namespace material {

class Sampler_cache {
public:

	Sampler_cache();
	~Sampler_cache();

	const image::texture::sampler::Sampler_2D& sampler(uint32_t key, Sampler_settings::Filter filter) const;

private:

	image::texture::sampler::Sampler_2D* samplers_[2];
};

}}
