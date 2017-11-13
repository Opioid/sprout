#include "null_light.hpp"
#include "light_sample.hpp"
#include "base/math/vector3.inl"

namespace scene::light {

const Light::Transformation& Null_light::transformation_at(
		float /*time*/, Transformation& transformation) const {
	return transformation;
}

bool Null_light::sample(const Transformation& /*transformation*/, const float3& /*p*/,
						const float3& /*n*/, float /*time*/, bool /*total_sphere*/,
						sampler::Sampler& /*sampler*/, uint32_t /*sampler_dimension*/,
						Sampler_filter /*filter*/, const Worker& /*worker*/,
						Sample& /*result*/) const {
	return false;
}

float Null_light::pdf(const Ray& /*ray*/, const Intersection& /*intersection*/,
					  bool /*total_sphere*/, Sampler_filter /*filter*/,
					  const Worker& /*worker*/) const {
	return 0.f;
}

float3 Null_light::power(const math::AABB& /*scene_bb*/) const {
	return float3::identity();
}

void Null_light::prepare_sampling(uint32_t /*light_id*/, thread::Pool& /*pool*/) {}

bool Null_light::equals(const Prop* /*prop*/, uint32_t /*part*/) const {
	return false;
}

}
