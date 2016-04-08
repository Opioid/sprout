#include "sky_sample_overcast.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace sky {

math::float3 Sample_overcast::evaluate(math::pfloat3 /*wi*/, float& pdf) const {
	pdf = 0.f;
	return math::float3_identity;
}

math::float3 Sample_overcast::emission() const {
	return ((1.f + 2.f * math::dot(zenith_, -wo_)) / 3.f) * emission_;
}

math::float3 Sample_overcast::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

float Sample_overcast::ior() const {
	return 1.5f;
}

void Sample_overcast::sample_evaluate(sampler::Sampler& /*sampler*/, bxdf::Result& result) const {
	result.reflection = math::float3_identity;
	result.pdf = 0.f;
}

bool Sample_overcast::is_pure_emissive() const {
	return true;
}

bool Sample_overcast::is_transmissive() const {
	return false;
}

bool Sample_overcast::is_translucent() const {
	return false;
}

void Sample_overcast::set(math::pfloat3 zenith, math::pfloat3 emission) {
	zenith_   = zenith;
	emission_ = emission;
}

}}}
