#ifndef SU_CORE_SCENE_MATERIAL_COATING_MATERIAL_INL
#define SU_CORE_SCENE_MATERIAL_COATING_MATERIAL_INL

#include "scene/material/material_helper.hpp"
#include "scene/scene_renderstate.hpp"
#include "substitute_base_material.inl"
#include "substitute_coating_material.hpp"

namespace scene::material::substitute {

template <typename Coating>
Material_coating<Coating>::Material_coating(Sampler_settings sampler_settings, bool two_sided)
    : Material_base(sampler_settings, two_sided) {}

template <typename Coating>
void Material_coating<Coating>::set_coating_thickness_map(Texture const& thickness_map) {
    coating_thickness_map_ = thickness_map;
}

template <typename Coating>
void Material_coating<Coating>::set_coating_normal_map(Texture const& normal_map) {
    coating_normal_map_ = normal_map;
}

template <typename Coating>
void Material_coating<Coating>::set_coating_thickness(float thickness) {
    coating_.thickness = thickness;
}

template <typename Coating>
template <typename Sample>
void Material_coating<Coating>::set_coating_basis(float3_p wo, Renderstate const& rs,
                                                  Texture_sampler_2D const& sampler,
                                                  Worker const& worker, Sample& sample) const {
    if (Material_base::normal_map_ == coating_normal_map_) {
        sample.coating_.set_tangent_frame(sample.layer_.t_, sample.layer_.b_, sample.layer_.n_);
    } else if (coating_normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, coating_normal_map_, sampler, worker.scene());
        sample.coating_.set_tangent_frame(n);
    } else {
        sample.coating_.set_tangent_frame(rs.t, rs.b, rs.n);
    }
}

}  // namespace scene::material::substitute

#endif
