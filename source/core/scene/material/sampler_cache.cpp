#include "sampler_cache.hpp"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/address_mode.hpp"

namespace scene { namespace material {

Sampler_cache::Sampler_cache() {
	using namespace image::texture::sampler;

	samplers_[0] = new Sampler_2D_nearest<Address_mode_repeat>;
	samplers_[1] = new Sampler_2D_linear <Address_mode_repeat>;
}

Sampler_cache::~Sampler_cache() {
	delete samplers_[0];
	delete samplers_[1];
}

const image::texture::sampler::Sampler_2D&
Sampler_cache::sampler(uint32_t key, Sampler_settings::Filter filter) const {
	if (Sampler_settings::Filter::Unknown == filter) {
		return *samplers_[key];
	} else {
		return *samplers_[static_cast<uint32_t>(filter)];
	}
}

}}
