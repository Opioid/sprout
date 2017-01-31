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

	samplers_2D_[0] = new Nearest_2D<Address_mode_clamp,  Address_mode_clamp>;
	samplers_2D_[1] = new Nearest_2D<Address_mode_clamp,  Address_mode_repeat>;
	samplers_2D_[2] = new Nearest_2D<Address_mode_repeat, Address_mode_clamp>;
	samplers_2D_[3] = new Nearest_2D<Address_mode_repeat, Address_mode_repeat>;
	samplers_2D_[4] = new Linear_2D <Address_mode_clamp,  Address_mode_clamp>;
	samplers_2D_[5] = new Linear_2D <Address_mode_clamp,  Address_mode_repeat>;
	samplers_2D_[6] = new Linear_2D <Address_mode_repeat, Address_mode_clamp>;
	samplers_2D_[7] = new Linear_2D <Address_mode_repeat, Address_mode_repeat>;

	samplers_3D_[0] = new Nearest_3D<Address_mode_clamp/*,  Address_mode_clamp*/>;
	samplers_3D_[1] = new Nearest_3D<Address_mode_clamp/*,  Address_mode_repeat*/>;
	samplers_3D_[2] = new Nearest_3D<Address_mode_repeat/*, Address_mode_clamp*/>;
	samplers_3D_[3] = new Nearest_3D<Address_mode_repeat/*, Address_mode_repeat*/>;
	samplers_3D_[4] = new Linear_3D <Address_mode_clamp/*,  Address_mode_clamp*/>;
	samplers_3D_[5] = new Linear_3D <Address_mode_clamp/*,  Address_mode_repeat*/>;
	samplers_3D_[6] = new Linear_3D <Address_mode_repeat/*, Address_mode_clamp*/>;
	samplers_3D_[7] = new Linear_3D <Address_mode_repeat/*, Address_mode_repeat*/>;
}

Sampler_cache::~Sampler_cache() {
	for (uint32_t i = 0; i < Num_samplers; ++i) {
		delete samplers_2D_[i];
		delete samplers_3D_[i];
	}
}

const Texture_sampler_2D& Sampler_cache::sampler_2D(uint32_t key, Sampler_filter filter) const {
	if (Sampler_filter::Unknown == filter) {
		return *samplers_2D_[key];
	} else {
		uint32_t address = key & static_cast<uint32_t>(Sampler_settings::Address_flat::Mask);
		uint32_t override_key = static_cast<uint32_t>(filter) | address;
		return *samplers_2D_[override_key];
	}
}

const Texture_sampler_3D& Sampler_cache::sampler_3D(uint32_t key, Sampler_filter filter) const {
	if (Sampler_filter::Unknown == filter) {
		return *samplers_3D_[key];
	} else {
		uint32_t address = key & static_cast<uint32_t>(Sampler_settings::Address_flat::Mask);
		uint32_t override_key = static_cast<uint32_t>(filter) | address;
		return *samplers_3D_[override_key];
	}
}

}}
