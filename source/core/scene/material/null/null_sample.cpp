#include "null_sample.hpp"
#include "scene/material/bxdf.hpp"
#include "base/math/vector3.inl"

namespace scene::material::null {

const material::Sample::Layer& Sample::base_layer() const {
	return layer_;
}

bxdf::Result Sample::evaluate(f_float3 /*wi*/) const {
	return { float3(0.f), 0.f };
}

void Sample::sample(sampler::Sampler& /*sampler*/, bxdf::Sample& result) const {
	result.reflection = float3(1.f);
	result.wi = -wo_;
	result.pdf = 1.f;
	result.wavelength = 0.f;
	result.type.clear(bxdf::Type::Transmission);
}

bool Sample::ior_greater_one() const {
	return false;
}

}
