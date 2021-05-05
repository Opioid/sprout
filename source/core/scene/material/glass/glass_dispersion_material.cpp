#include "glass_dispersion_material.hpp"
#include "base/math/vector4.inl"
#include "glass_dispersion_sample.hpp"

#include "scene/material/material.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_ray.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::glass {

Glass_dispersion::Glass_dispersion(Sampler_settings sampler_settings) : Glass(sampler_settings) {}

material::Sample const& Glass_dispersion::sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                                 Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample_dispersion>();

    if (normal_map_.is_valid()) {
        auto& sampler = worker.sampler_2D(sampler_key(), rs.filter);

        float3 const n = sample_normal(wo, rs, normal_map_, sampler, worker.scene());
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.set_common(rs, wo, refraction_color_, float3(0.f), rs.alpha);
    sample.set(ior_, rs.ior);
    sample.set_dispersion(abbe_, ray.wavelength);

    return sample;
}

void Glass_dispersion::set_abbe(float abbe) {
    abbe_ = abbe;
}

size_t Glass_dispersion::sample_size() {
    return sizeof(Sample_dispersion);
}

}  // namespace scene::material::glass
