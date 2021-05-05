#include "volumetric_homogeneous.hpp"
#include "base/math/ray.inl"
#include "scene/material/material_sample_helper.hpp"

namespace scene::material::volumetric {

Homogeneous::Homogeneous(Sampler_settings sampler_settings) : Material(sampler_settings) {}

void Homogeneous::commit(Threads& /*threads*/, Scene const& /*scene*/) {
    properties_.set(Property::Scattering_volume,
                    any_greater_zero(cc_.s) || any_greater_zero(emission_));
}

float3 Homogeneous::evaluate_radiance(float3_p /*wi*/, float3_p /*n*/, float3_p /*uvw*/,
                                      float /*volume*/, Filter /*filter*/,
                                      Worker const& /*worker*/) const {
    return cc_.a * emission_;
}

}  // namespace scene::material::volumetric
