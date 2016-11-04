#include "sampler_cache.hpp"
#include "image/texture/sampler/sampler_2d_nearest.inl"
#include "image/texture/sampler/sampler_2d_linear.inl"
#include "image/texture/sampler/sampler_3d_nearest.inl"
#include "image/texture/sampler/sampler_3d_linear.inl"
#include "image/texture/sampler/address_mode.hpp"

namespace scene { namespace material {

Sampler_cache::Sampler_cache() {
	using namespace image::texture::sampler;

	samplers_2D_[0] = new Sampler_2d_nearest<Address_mode_repeat>;
	samplers_2D_[1] = new Sampler_2d_linear <Address_mode_repeat>;

	samplers_3D_[0] = new Sampler_3d_nearest<Address_mode_clamp>;
	samplers_3D_[1] = new Sampler_3d_linear <Address_mode_clamp>;
}

Sampler_cache::~Sampler_cache() {
	delete samplers_3D_[0];
	delete samplers_3D_[1];

	delete samplers_2D_[0];
	delete samplers_2D_[1];
}

const image::texture::sampler::Sampler_2d&
Sampler_cache::sampler_2D(uint32_t key, Sampler_settings::Filter filter) const {
	if (Sampler_settings::Filter::Unknown == filter) {
		return *samplers_2D_[key];
	} else {
		return *samplers_2D_[static_cast<uint32_t>(filter)];
	}
}

const image::texture::sampler::Sampler_3d&
Sampler_cache::sampler_3D(uint32_t key, Sampler_settings::Filter filter) const {
	if (Sampler_settings::Filter::Unknown == filter) {
		return *samplers_3D_[key];
	} else {
		return *samplers_3D_[static_cast<uint32_t>(filter)];
	}
}

}}
