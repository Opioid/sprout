#pragma once

#include "texture_filter.hpp"
#include <cstdint>

namespace image { namespace texture { namespace sampler { class Sampler_2D; }}}

namespace scene { namespace material {

class Texture_sampler_cache {
public:

	Texture_sampler_cache();
	~Texture_sampler_cache();

	const image::texture::sampler::Sampler_2D& sampler(uint32_t key, Texture_filter override_filter) const;

private:

	image::texture::sampler::Sampler_2D* samplers_[2];
};

}}
