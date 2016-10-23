#pragma once

#include "sampler_settings.hpp"
#include <cstdint>

namespace image { namespace texture { namespace sampler {
	class Sampler_2d;
	class Sampler_3d;
}}}

namespace scene { namespace material {

class Sampler_cache {

public:

	Sampler_cache();
	~Sampler_cache();

	using Sampler_2d = image::texture::sampler::Sampler_2d;

	using Sampler_3d = image::texture::sampler::Sampler_3d;

	const Sampler_2d& sampler_2D(uint32_t key, Sampler_settings::Filter filter) const;

	const Sampler_3d& sampler_3D(uint32_t key, Sampler_settings::Filter filter) const;

private:

	Sampler_2d* samplers_2D_[2];
	Sampler_3d* samplers_3D_[2];
};

}}
