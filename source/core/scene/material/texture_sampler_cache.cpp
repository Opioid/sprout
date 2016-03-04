#include "texture_sampler_cache.hpp"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/address_mode.hpp"

namespace scene { namespace material {

Texture_sampler_cache::Texture_sampler_cache() {
	samplers_[0] = new image::texture::sampler::Sampler_2D_nearest<image::texture::sampler::Address_mode_repeat>;
	samplers_[1] = new image::texture::sampler::Sampler_2D_linear <image::texture::sampler::Address_mode_repeat>;
}

Texture_sampler_cache::~Texture_sampler_cache() {
	delete samplers_[0];
	delete samplers_[1];
}

const image::texture::sampler::Sampler_2D&
Texture_sampler_cache::sampler(uint32_t key, Texture_filter override_filter) const {
	if (Texture_filter::Unknown == override_filter) {
		return *samplers_[key];
	} else {
		return *samplers_[static_cast<uint32_t>(override_filter)];
	}
}

}}
