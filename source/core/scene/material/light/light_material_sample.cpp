#include "light_material_sample.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::light {

Sample::Sample() {
    properties_.set(Property::Pure_emissive);
}

bxdf::Result Sample::evaluate_f(float3 const& /*wi*/) const {
    return {float3(0.f), 0.f};
}

bxdf::Result Sample::evaluate_b(float3 const& /*wi*/) const {
    return {float3(0.f), 0.f};
}

void Sample::sample(Sampler& /*sampler*/, rnd::Generator& /*rng*/, bxdf::Sample& result) const {
    result.reflection = float3(0.f);
    result.pdf        = 0.f;
}

}  // namespace scene::material::light
