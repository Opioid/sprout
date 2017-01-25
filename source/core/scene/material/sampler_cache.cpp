#include "sampler_cache.hpp"
#include "image/texture/sampler/sampler_linear_2d.inl"
#include "image/texture/sampler/sampler_nearest_2d.inl"
#include "image/texture/sampler/sampler_linear_3d.inl"
#include "image/texture/sampler/sampler_nearest_3d.inl"
#include "image/texture/sampler/address_mode.hpp"

namespace scene { namespace material {

using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
using Texture_sampler_3D = image::texture::sampler::Sampler_3D;

Sampler_cache::Sampler_cache() {
	using namespace image::texture::sampler;

	samplers_2D_[0] = new Nearest_2D<Address_mode_clamp>;
	samplers_2D_[1] = new Nearest_2D<Address_mode_repeat>;
	samplers_2D_[2] = new Linear_2D <Address_mode_clamp>;
	samplers_2D_[3] = new Linear_2D <Address_mode_repeat>;

	samplers_3D_[0] = new Nearest_3D<Address_mode_clamp>;
	samplers_3D_[1] = new Linear_3D <Address_mode_clamp>;
}

Sampler_cache::~Sampler_cache() {
	delete samplers_3D_[0];
	delete samplers_3D_[1];

	delete samplers_2D_[0];
	delete samplers_2D_[1];
	delete samplers_2D_[2];
	delete samplers_2D_[3];
}

const Texture_sampler_2D& Sampler_cache::sampler_2D(uint32_t key, Sampler_filter filter) const {
	if (Sampler_filter::Unknown == filter) {
		return *samplers_2D_[key];
	} else {
		uint32_t address = key & 0x00000001;
		uint32_t override_key = static_cast<uint32_t>(filter) << 1 | address;
		return *samplers_2D_[override_key];
	}
}

const Texture_sampler_3D& Sampler_cache::sampler_3D(uint32_t key, Sampler_filter filter) const {
	if (Sampler_filter::Unknown == filter) {
		return *samplers_3D_[key];
	} else {
		return *samplers_3D_[static_cast<uint32_t>(filter)];
	}
}

}}
