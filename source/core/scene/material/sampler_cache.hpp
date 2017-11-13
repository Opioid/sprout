#ifndef SU_CORE_SCENE_MATERIAL_SAMPLER_CACHE_HPP
#define SU_CORE_SCENE_MATERIAL_SAMPLER_CACHE_HPP

#include "sampler_settings.hpp"
#include <cstdint>

namespace image::texture::sampler {
	class Sampler_2D;
	class Sampler_3D;
}

namespace scene::material {

class Sampler_cache {

public:

	using Sampler_filter = Sampler_settings::Filter;
	using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
	using Texture_sampler_3D = image::texture::sampler::Sampler_3D;

	Sampler_cache();
	~Sampler_cache();

	const Texture_sampler_2D& sampler_2D(uint32_t key, Sampler_filter filter) const;

	const Texture_sampler_3D& sampler_3D(uint32_t key, Sampler_filter filter) const;

private:

	static constexpr uint32_t Num_samplers = 8;

	Texture_sampler_2D* samplers_2D_[Num_samplers];
	Texture_sampler_3D* samplers_3D_[Num_samplers];
};

}

#endif
