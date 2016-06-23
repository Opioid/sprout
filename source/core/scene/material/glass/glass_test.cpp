#include "glass_test.hpp"
#include "glass_sample.hpp"
#include "scene/material/material_print.hpp"
#include "sampler/sampler_random.hpp"
#include "base/math/random/generator.hpp"

namespace scene { namespace material { namespace glass {

namespace testing {

void test() {
	math::random::Generator rng;
	sampler::Random sampler(rng, 0);

	bxdf::Result result;

	Sample sample;

	float3 color(1.f, 1.f, 1.f);
	float attenuation_distance = 1.f;
	float ior = 1.5f;

	sample.set(color, attenuation_distance, ior, 1.f);

	float3 t(1.f, 0.f, 0.f);
	float3 b(0.f, 1.f, 0.f);
	float3 n(0.f, 0.f, 1.f);

	float3 wo;

	wo = t;
	sample.set_basis(t, b, n, n, wo);
	result.wi = math::float3_identity;
	result.reflection = math::float3_identity;
	BRDF::importance_sample(sample, sampler, result);
	print(result);
	result.reflection = math::float3_identity;
	BTDF::importance_sample(sample, sampler, result);
	print(result);

	wo = b;
	sample.set_basis(t, b, n, n, wo);
	result.reflection = math::float3_identity;
	BRDF::importance_sample(sample, sampler, result);
	print(result);
	result.reflection = math::float3_identity;
	BTDF::importance_sample(sample, sampler, result);
	print(result);

	wo = n;
	sample.set_basis(t, b, n, n, wo);
	result.reflection = math::float3_identity;
	BRDF::importance_sample(sample, sampler, result);
	print(result);
	result.reflection = math::float3_identity;
	BTDF::importance_sample(sample, sampler, result);
	print(result);
}

}

}}}
