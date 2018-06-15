#include "null_sample.hpp"
#include "base/math/vector3.inl"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::null {

const material::Sample::Layer& Sample::base_layer() const {
    return layer_;
}

bxdf::Result Sample::evaluate(f_float3 /*wi*/, bool /*avoid_caustics*/) const {
    return {float3::identity(), 0.f};
}

void Sample::sample(sampler::Sampler& /*sampler*/, bool /*avoid_caustics*/,
                    bxdf::Sample& result) const {
    result.reflection = float3(1.f);
    result.wi         = -wo_;
    result.pdf        = 1.f;
    result.wavelength = 0.f;
    result.type.clear(bxdf::Type::Transmission);
}

bool Sample::ior_greater_one() const {
    return false;
}

}  // namespace scene::material::null
