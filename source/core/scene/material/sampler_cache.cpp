#include "sampler_cache.hpp"
#include "image/texture/sampler/address_mode.hpp"
#include "image/texture/sampler/sampler_linear_2d.inl"
#include "image/texture/sampler/sampler_linear_3d.inl"
#include "image/texture/sampler/sampler_nearest_2d.inl"
#include "image/texture/sampler/sampler_nearest_3d.inl"

namespace scene::material {

using Texture_sampler_2D = image::texture::sampler::Sampler_2D;
using Texture_sampler_3D = image::texture::sampler::Sampler_3D;

Sampler_cache::Sampler_cache() noexcept {
    using namespace image::texture::sampler;

    samplers_2D_[0] = new Nearest_2D<Address_mode_clamp, Address_mode_clamp>;
    samplers_2D_[1] = new Nearest_2D<Address_mode_clamp, Address_mode_repeat>;
    samplers_2D_[2] = new Nearest_2D<Address_mode_repeat, Address_mode_clamp>;
    samplers_2D_[3] = new Nearest_2D<Address_mode_repeat, Address_mode_repeat>;
    samplers_2D_[4] = new Linear_2D<Address_mode_clamp, Address_mode_clamp>;
    samplers_2D_[5] = new Linear_2D<Address_mode_clamp, Address_mode_repeat>;
    samplers_2D_[6] = new Linear_2D<Address_mode_repeat, Address_mode_clamp>;
    samplers_2D_[7] = new Linear_2D<Address_mode_repeat, Address_mode_repeat>;

    samplers_3D_[0] = new Nearest_3D<Address_mode_clamp /*,  Address_mode_clamp*/>;
    samplers_3D_[1] = new Nearest_3D<Address_mode_clamp /*,  Address_mode_repeat*/>;
    samplers_3D_[2] = new Nearest_3D<Address_mode_repeat /*, Address_mode_clamp*/>;
    samplers_3D_[3] = new Nearest_3D<Address_mode_repeat /*, Address_mode_repeat*/>;
    samplers_3D_[4] = new Linear_3D<Address_mode_clamp /*,  Address_mode_clamp*/>;
    samplers_3D_[5] = new Linear_3D<Address_mode_clamp /*,  Address_mode_repeat*/>;
    samplers_3D_[6] = new Linear_3D<Address_mode_repeat /*, Address_mode_clamp*/>;
    samplers_3D_[7] = new Linear_3D<Address_mode_repeat /*, Address_mode_repeat*/>;
}

Sampler_cache::~Sampler_cache() noexcept {
    for (uint32_t i = 0; i < Num_samplers; ++i) {
        delete samplers_2D_[i];
        delete samplers_3D_[i];
    }
}

Texture_sampler_2D const& Sampler_cache::sampler_2D(uint32_t key, Filter filter) const noexcept {
    if (Filter::Undefined == filter) {
        return *samplers_2D_[key];
    } else {
        uint32_t const address = key & static_cast<uint32_t>(Sampler_settings::Address_flat::Mask);
        uint32_t const override_key = static_cast<uint32_t>(filter) | address;
        return *samplers_2D_[override_key];
    }
}

Texture_sampler_3D const& Sampler_cache::sampler_3D(uint32_t key, Filter filter) const noexcept {
    if (Filter::Undefined == filter) {
        return *samplers_3D_[key];
    } else {
        uint32_t const address = key & static_cast<uint32_t>(Sampler_settings::Address_flat::Mask);
        uint32_t const override_key = static_cast<uint32_t>(filter) | address;
        return *samplers_3D_[override_key];
    }
}

}  // namespace scene::material
