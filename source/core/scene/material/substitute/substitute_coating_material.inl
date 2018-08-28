#ifndef SU_CORE_SCENE_MATERIAL_COATING_MATERIAL_INL
#define SU_CORE_SCENE_MATERIAL_COATING_MATERIAL_INL

#include "image/texture/texture_adapter.inl"
#include "scene/material/material_helper.hpp"
#include "scene/scene_renderstate.hpp"
#include "substitute_base_material.inl"
#include "substitute_coating_material.hpp"

namespace scene::material::substitute {

template <typename Coating>
Material_coating<Coating>::Material_coating(Sampler_settings const& sampler_settings,
                                            bool                    two_sided) noexcept
    : Material_base(sampler_settings, two_sided) {}

template <typename Coating>
size_t Material_coating<Coating>::num_bytes() const noexcept {
    return sizeof(*this);
}

template <typename Coating>
void Material_coating<Coating>::set_coating_weight_map(Texture_adapter const& weight_map) noexcept {
    coating_weight_map_ = weight_map;
}

template <typename Coating>
void Material_coating<Coating>::set_coating_normal_map(Texture_adapter const& normal_map) noexcept {
    coating_normal_map_ = normal_map;
}

template <typename Coating>
void Material_coating<Coating>::set_coating_thickness(float thickness) noexcept {
    coating_.thickness_ = thickness;
}

template <typename Coating>
template <typename Sample>
void Material_coating<Coating>::set_coating_basis(float3 const& wo, Renderstate const& rs,
                                                  Texture_sampler_2D const& sampler,
                                                  Sample&                   sample) const noexcept {
    if (Material_base::normal_map_ == coating_normal_map_) {
        sample.coating_.set_tangent_frame(sample.layer_.t_, sample.layer_.b_, sample.layer_.n_);
    } else if (coating_normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler);
        sample.coating_.set_tangent_frame(n);
    } else {
        sample.coating_.set_tangent_frame(rs.t, rs.b, rs.n);
    }
}

}  // namespace scene::material::substitute

#endif
