#include "glass_test.hpp"
#include "glass.hpp"
#include "sampler/random_sampler.hpp"
#include "base/math/random/generator.hpp"
#include "base/math/print.hpp"
#include <iostream>

namespace scene { namespace material { namespace glass {

namespace test {

void print(const BxDF_result& result);

void test() {
	math::random::Generator rng;
	sampler::Random sampler(rng, 0);

	BxDF_result result;

	Sample sample;
	BRDF brdf(sample);
	BTDF btdf(sample);

	math::float3 color(1.f, 1.f, 1.f);

	float attenuation_distance = 1.f;
	math::float3 attenuation(1.f / (color.x * attenuation_distance),
							 1.f / (color.y * attenuation_distance),
							 1.f / (color.z * attenuation_distance));

	float ior = 1.5f;

	sample.set(color, attenuation, ior);

	math::float3 t(1.f, 0.f, 0.f);
	math::float3 b(0.f, 1.f, 0.f);
	math::float3 n(0.f, 0.f, 1.f);

	math::float3 wo;

	wo = t;
	sample.set_basis(t, b, n, n, wo);
	result.wi = math::float3::identity;
	result.reflection = math::float3::identity;
	brdf.importance_sample(sampler, result);
	print(result);
	result.reflection = math::float3::identity;
	btdf.importance_sample(sampler, result);
	print(result);

	wo = b;
	sample.set_basis(t, b, n, n, wo);
	result.reflection = math::float3::identity;
	brdf.importance_sample(sampler, result);
	print(result);
	result.reflection = math::float3::identity;
	btdf.importance_sample(sampler, result);
	print(result);

	wo = n;
	sample.set_basis(t, b, n, n, wo);
	result.reflection = math::float3::identity;
	brdf.importance_sample(sampler, result);
	print(result);
	result.reflection = math::float3::identity;
	btdf.importance_sample(sampler, result);
	print(result);
}

void print(const BxDF_result& result) {
	std::cout << "wi " << result.wi << std::endl;
	std::cout << "reflection " << result.reflection << std::endl;
}


}

}}}
