#include "light_material_sample.hpp"
#include "sampler/sampler.hpp"
#include "base/math/vector.inl"

namespace scene { namespace material { namespace light {

math::vec3 Sample::evaluate(math::pvec3 /*wi*/, float& pdf) const {
	pdf = 0.f;
	return math::vec3_identity;
}

math::vec3 Sample::emission() const {
	return emission_;
}

math::vec3 Sample::attenuation() const {
	return math::vec3(100.f, 100.f, 100.f);
}

float Sample::ior() const {
	return 1.5f;
}

void Sample::sample_evaluate(sampler::Sampler& /*sampler*/, bxdf::Result& result) const {
	result.reflection = math::vec3_identity;
	result.pdf = 0.f;
}

bool Sample::is_pure_emissive() const {
	return true;
}

bool Sample::is_transmissive() const {
	return false;
}

bool Sample::is_translucent() const {
	return false;
}

void Sample::set(const math::vec3& emission) {
	emission_ = emission;
}

}}}
