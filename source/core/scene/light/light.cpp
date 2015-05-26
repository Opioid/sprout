#include "light.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene { namespace light {

void Light::sample(float time, const math::float3& p, uint32_t max_samples, sampler::Sampler& sampler, std::vector<Sample>& samples) const {
	Composed_transformation transformation;
	transformation_at(time, transformation);

	sample(transformation, p, max_samples, sampler, samples);
};

}}

