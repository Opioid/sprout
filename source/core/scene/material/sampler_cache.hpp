#pragma once

#include "sampler_settings.hpp"
#include <cstdint>

namespace image { namespace texture { namespace sampler {
	class Sampler_2D;
	class Sampler_3D;
}}}

namespace scene { namespace material {

class Sampler_cache {

public:

	Sampler_cache();
	~Sampler_cache();

	using Sampler_2D = image::texture::sampler::Sampler_2D;

	using Sampler_3D = image::texture::sampler::Sampler_3D;

	const Sampler_2D& sampler_2D(uint32_t key, Sampler_settings::Filter filter) const;

	const Sampler_3D& sampler_3D(uint32_t key, Sampler_settings::Filter filter) const;

private:

	Sampler_2D* samplers_2D_[2];
	Sampler_3D* samplers_3D_[2];
};

}}
