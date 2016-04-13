#include "sky_sample_clear.hpp"
#include "sky_model.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace sky {

math::float3 Sample_clear::evaluate(math::pfloat3 /*wi*/, float& pdf) const {
	pdf = 0.f;
	return math::float3_identity;
}

math::float3 Sample_clear::emission() const {
	return model_->evaluate(-wo_);
}

math::float3 Sample_clear::attenuation() const {
	return math::float3(100.f, 100.f, 100.f);
}

float Sample_clear::ior() const {
	return 1.5f;
}

void Sample_clear::sample_evaluate(sampler::Sampler& /*sampler*/, bxdf::Result& result) const {
	result.reflection = math::float3_identity;
	result.pdf = 0.f;
}

bool Sample_clear::is_pure_emissive() const {
	return true;
}

bool Sample_clear::is_transmissive() const {
	return false;
}

bool Sample_clear::is_translucent() const {
	return false;
}

void Sample_clear::set(const Model* model, math::pfloat3 zenith) {
	model_ = model;
	zenith_   = zenith;
}

}}}
