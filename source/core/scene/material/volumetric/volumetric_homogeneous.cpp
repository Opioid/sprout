#include "volumetric_homogeneous.hpp"
#include "base/math/ray.inl"
#include "scene/material/material_sample_helper.hpp"

namespace scene::material::volumetric {

Homogeneous::Homogeneous(Sampler_settings const& sampler_settings) : Material(sampler_settings) {}

float3 Homogeneous::emission(math::Ray const& /*ray*/, Transformation const& /*transformation*/,
                             float /*step_size*/, rnd::Generator& /*rng*/,
                             Sampler_filter /*filter*/, Worker const& /*worker*/) const {
    return float3::identity();
}

float3 Homogeneous::absorption_coefficient(float2 /*uv*/, Sampler_filter /*filter*/,
                                           Worker const& /*worker*/) const {
    return cc_.a;
}

CC Homogeneous::collision_coefficients() const {
    return cc_;
}

CC Homogeneous::collision_coefficients(float2 /*uv*/, Sampler_filter /*filter*/,
                                       Worker const& /*worker*/) const {
    return cc_;
}

CC Homogeneous::collision_coefficients(f_float3 /*p*/, Sampler_filter /*filter*/,
                                       Worker const& /*worker*/) const {
    return cc_;
}

float Homogeneous::majorant_mu_t() const {
    return math::max_component(cc_.a + cc_.s);
}

size_t Homogeneous::num_bytes() const {
    return sizeof(*this);
}

}  // namespace scene::material::volumetric
