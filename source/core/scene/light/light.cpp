#include "light.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "base/math/vector.inl"
#include "base/math/matrix.inl"

namespace scene { namespace light {

void Light::sample(float time, const math::float3& p, const math::float3& n,
				   const image::sampler::Sampler_2D& image_sampler, sampler::Sampler& sampler,
				   uint32_t max_samples, std::vector<Sample>& samples) const {
	Composed_transformation transformation;
	transformation_at(time, transformation);

	sample(transformation, p, n, image_sampler, sampler, max_samples, samples);
};

}}

