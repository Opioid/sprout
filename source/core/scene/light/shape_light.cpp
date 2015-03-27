#include "shape_light.hpp"
#include "light_sample.hpp"
#include "base/math/vector.inl"

namespace scene { namespace light {

void Shape_light::sample(const math::float3& p, float time, uint32_t max_samples, rendering::sampler::Sampler& sampler,
						 std::vector<Sample>& samples) const {
	Composed_transformation transformation;
	transformation_at(time, transformation);

	samples.clear();

	Sample sample;

	math::float3 v = transformation.position - p;
	float d = math::squared_length(v);
	float i = 1.f / d;
	float t = std::sqrt(d);

	sample.l = v / t;
	sample.energy = i * lumen_ * color_;
	sample.t = t;
	sample.pdf = 1.f;

	samples.push_back(sample);
}

void Shape_light::set_color(const math::float3& color) {
	color_ = color;
}

void Shape_light::set_lumen(float lumen) {
	lumen_ = lumen;
}

}}
